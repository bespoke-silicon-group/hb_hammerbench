#include <bsg_manycore_cuda.h>
#include <bsg_manycore_errno.h>
#include <bsg_manycore_loader.h>
#include <bsg_manycore_regression.h>
#include <bsg_manycore_tile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ldpc_golden.hpp"

#define ALLOC_NAME "default_allocator"

#define BG_ROWS 46
#define BG_COLS 68

#ifndef SIZE
Please define SIZE in Makefile.
#endif

#ifndef Z_FACTOR
#define Z_FACTOR SIZE
#endif

static int read_ints_from_file(const char* filename, int* buf, int count) {
  FILE* f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "Failed to open %s\n", filename);
    return -1;
  }
  for (int i = 0; i < count; i++) {
    if (fscanf(f, "%d", &buf[i]) != 1) {
      fprintf(stderr, "Failed to read element %d from %s\n", i, filename);
      fclose(f);
      return -1;
    }
  }
  fclose(f);
  return 0;
}

int kernel_LDPC(int argc, char** argv) {

  char *bin_path, *test_name;
  struct arguments_path args = {NULL, NULL};

  argp_parse(&argp_path, argc, argv, 0, 0, &args);
  bin_path = args.path;
  test_name = args.name;

  bsg_pr_test_info("Running kernel_LDPC.\n");

  hb_mc_device_t device;
  BSG_CUDA_CALL(hb_mc_device_init(&device, test_name, HB_MC_DEVICE_ID));

  hb_mc_pod_id_t pod;
  hb_mc_device_foreach_pod_id(&device, pod) {
    bsg_pr_info("Loading program for pod %d.\n", pod);
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));
    BSG_CUDA_CALL(hb_mc_device_program_init(&device, bin_path, ALLOC_NAME, 0));

    int z = Z_FACTOR;
    int cols = BG_COLS * z;
    int bg_size = BG_ROWS * BG_COLS;

    bsg_pr_info("LDPC: z=%d, bg=%dx%d, cols=%d\n", z, BG_ROWS, BG_COLS, cols);

    // Read base graph B
    int* B_host = (int*)malloc(sizeof(int) * bg_size);
    char bg_filename[256];
    snprintf(bg_filename, sizeof(bg_filename), "%s/NR_1_0_128.txt", APP_PATH);
    if (read_ints_from_file(bg_filename, B_host, bg_size) != 0) {
      bsg_pr_err("Failed to read base graph from %s\n", bg_filename);
      free(B_host);
      BSG_CUDA_CALL(hb_mc_device_finish(&device));
      return HB_MC_FAIL;
    }

    // Count edges in base graph
    int num_edges = 0;
    for (int i = 0; i < bg_size; i++)
      if (B_host[i] != -1) num_edges++;
    bsg_pr_info("LDPC: num_edges=%d, R size=%d ints\n", num_edges,
                num_edges * z);

    // Read channel LLR (two's complement int4)
    int* r_host = (int*)malloc(sizeof(int) * cols);
    char llr_filename[256];
    snprintf(llr_filename, sizeof(llr_filename), "%s/LLRs_q_z%d_int4.txt",
             APP_PATH, z);
    if (read_ints_from_file(llr_filename, r_host, cols) != 0) {
      bsg_pr_err("Failed to read LLR values from %s\n", llr_filename);
      free(B_host);
      free(r_host);
      BSG_CUDA_CALL(hb_mc_device_finish(&device));
      return HB_MC_FAIL;
    }

    // Run golden decoder on host
    bsg_pr_info("Running C++ golden decoder...\n");
    int* decoded_expected = (int*)malloc(sizeof(int) * cols);
    int conv_iter =
        golden_decode(B_host, r_host, BG_ROWS, BG_COLS, z, 100, decoded_expected);
    bsg_pr_info("Golden decoder finished after %d iterations\n", conv_iter);

    int golden_errors = 0;
    for (int j = 0; j < cols; j++)
      golden_errors += (decoded_expected[j] != 0);

    // Device memory allocation
    int rnew_size = 68 * z;  // MAX_ROW_WEIGHT * z
    int num_tiles_hw = bsg_tiles_X * bsg_tiles_Y;
    int tiles_per_z = (z < num_tiles_hw) ? num_tiles_hw / z : 1;
    int partial_size = z * tiles_per_z * 4;  // 4 fields per (z_pos, edge_rank)
    eva_t B_device, r_device, L_device, R_device, Rnew_device, hard_device,
        partial_device;
    BSG_CUDA_CALL(
        hb_mc_device_malloc(&device, bg_size * sizeof(int), &B_device));
    BSG_CUDA_CALL(
        hb_mc_device_malloc(&device, cols * sizeof(int), &r_device));
    BSG_CUDA_CALL(
        hb_mc_device_malloc(&device, cols * sizeof(int), &L_device));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, num_edges * z * sizeof(int),
                                      &R_device));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, rnew_size * sizeof(int),
                                      &Rnew_device));
    BSG_CUDA_CALL(
        hb_mc_device_malloc(&device, cols * sizeof(int), &hard_device));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, partial_size * sizeof(int),
                                      &partial_device));

    // DMA host -> device
    hb_mc_dma_htod_t htod_job[] = {
        {.d_addr = B_device,
         .h_addr = (void*)B_host,
         .size = (size_t)(bg_size * sizeof(int))},
        {.d_addr = r_device,
         .h_addr = (void*)r_host,
         .size = (size_t)(cols * sizeof(int))}};
    BSG_CUDA_CALL(hb_mc_device_transfer_data_to_device(&device, htod_job, 2));

    // Kernel arguments
    hb_mc_dimension_t tg_dim = {.x = bsg_tiles_X, .y = bsg_tiles_Y};
    hb_mc_dimension_t grid_dim = {.x = 1, .y = 1};

#define CUDA_ARGC 11
    uint32_t cuda_argv[CUDA_ARGC] = {
        B_device,          r_device,          L_device,
        R_device,          Rnew_device,       hard_device,
        (uint32_t)BG_ROWS, (uint32_t)BG_COLS, (uint32_t)z,
        (uint32_t)num_edges, partial_device};

    BSG_CUDA_CALL(hb_mc_kernel_enqueue(&device, grid_dim, tg_dim, "kernel_LDPC",
                                       CUDA_ARGC, cuda_argv));

    hb_mc_manycore_trace_enable((&device)->mc);
    BSG_CUDA_CALL(hb_mc_device_tile_groups_execute(&device));
    hb_mc_manycore_trace_disable((&device)->mc);

    // DMA device -> host
    int* hard_kernel = (int*)malloc(sizeof(int) * cols);
    hb_mc_dma_dtoh_t dtoh_job[] = {{.d_addr = hard_device,
                                    .h_addr = (void*)hard_kernel,
                                    .size = (size_t)(cols * sizeof(int))}};
    BSG_CUDA_CALL(hb_mc_device_transfer_data_to_host(&device, dtoh_job, 1));

    // BER summary
    int kernel_errors = 0;
    for (int j = 0; j < cols; j++)
      kernel_errors += (hard_kernel[j] != 0);

    int mismatches = 0;
    for (int j = 0; j < cols; j++) {
      if (hard_kernel[j] != decoded_expected[j])
        mismatches++;
    }

    bsg_pr_info("=== BER Summary ===\n");
    bsg_pr_info("  Golden vs tx (all-zero):  %d / %d  (BER = %.6f)\n",
                golden_errors, cols, (double)golden_errors / cols);
    bsg_pr_info("  Kernel vs tx (all-zero):  %d / %d  (BER = %.6f)\n",
                kernel_errors, cols, (double)kernel_errors / cols);
    bsg_pr_info("  Kernel vs golden:         %d / %d\n", mismatches, cols);

    if (mismatches > 0) {
      for (int j = 0, printed = 0; j < cols && printed < 10; j++) {
        if (hard_kernel[j] != decoded_expected[j]) {
          bsg_pr_err("  MISMATCH [col %d]: kernel=%d, golden=%d\n", j,
                     hard_kernel[j], decoded_expected[j]);
          printed++;
        }
      }
      bsg_pr_err("FAIL: %d/%d bits differ between kernel and golden\n",
                 mismatches, cols);
      free(B_host);
      free(r_host);
      free(decoded_expected);
      free(hard_kernel);
      BSG_CUDA_CALL(hb_mc_device_finish(&device));
      return HB_MC_FAIL;
    }
    bsg_pr_test_info("PASS: all %d decoded bits match golden decoder\n", cols);

    free(B_host);
    free(r_host);
    free(decoded_expected);
    free(hard_kernel);

    BSG_CUDA_CALL(hb_mc_device_free(&device, B_device));
    BSG_CUDA_CALL(hb_mc_device_free(&device, r_device));
    BSG_CUDA_CALL(hb_mc_device_free(&device, L_device));
    BSG_CUDA_CALL(hb_mc_device_free(&device, R_device));
    BSG_CUDA_CALL(hb_mc_device_free(&device, Rnew_device));
    BSG_CUDA_CALL(hb_mc_device_free(&device, hard_device));
    BSG_CUDA_CALL(hb_mc_device_free(&device, partial_device));

    BSG_CUDA_CALL(hb_mc_device_program_finish(&device));
  }

  BSG_CUDA_CALL(hb_mc_device_finish(&device));
  return HB_MC_SUCCESS;
}

declare_program_main("LDPC", kernel_LDPC);
