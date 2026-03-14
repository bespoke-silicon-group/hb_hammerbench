#include <bsg_manycore_tile.h>
#include <bsg_manycore_errno.h>
#include <bsg_manycore_loader.h>
#include <bsg_manycore_cuda.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <bsg_manycore_regression.h>

#define ALLOC_NAME "default_allocator"
#define MAT_N 32

static void host_matmul_naive(float *A, float *B, float *C, int n) {
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++) {
      float sum = 0.0f;
      for (int k = 0; k < n; k++)
        sum += A[i*n+k] * B[k*n+j];
      C[i*n+j] = sum;
    }
}

int kernel_gemm(int argc, char **argv) {
  int rc;
  char *bin_path, *test_name;
  struct arguments_path args = {NULL, NULL};

  argp_parse(&argp_path, argc, argv, 0, 0, &args);
  bin_path = args.path;
  test_name = args.name;

  bsg_pr_test_info("Running GEMM dual-DMEM: %d x %d\n\n", MAT_N, MAT_N);

  int n = MAT_N;
  srand(42);

  hb_mc_device_t device;
  rc = hb_mc_device_init(&device, test_name, HB_MC_DEVICE_ID);
  if (rc != HB_MC_SUCCESS) return rc;

  rc = hb_mc_device_program_init(&device, bin_path, ALLOC_NAME, 0);
  if (rc != HB_MC_SUCCESS) return rc;

  hb_mc_eva_t A_device, B_device, C_device;
  rc = hb_mc_device_malloc(&device, n*n*sizeof(float), &A_device);
  if (rc != HB_MC_SUCCESS) return rc;
  rc = hb_mc_device_malloc(&device, n*n*sizeof(float), &B_device);
  if (rc != HB_MC_SUCCESS) return rc;
  rc = hb_mc_device_malloc(&device, n*n*sizeof(float), &C_device);
  if (rc != HB_MC_SUCCESS) return rc;

  float A_host[MAT_N * MAT_N];
  float B_host[MAT_N * MAT_N];
  for (int i = 0; i < n*n; i++) A_host[i] = (float)(rand() % 100) / 10.0f;
  for (int i = 0; i < n*n; i++) B_host[i] = (float)(rand() % 100) / 10.0f;

  rc = hb_mc_device_memcpy(&device, (void *)(intptr_t)A_device,
                           A_host, n*n*sizeof(float), HB_MC_MEMCPY_TO_DEVICE);
  if (rc != HB_MC_SUCCESS) return rc;

  rc = hb_mc_device_memcpy(&device, (void *)(intptr_t)B_device,
                           B_host, n*n*sizeof(float), HB_MC_MEMCPY_TO_DEVICE);
  if (rc != HB_MC_SUCCESS) return rc;

  rc = hb_mc_device_memset(&device, &C_device, 0, n*n*sizeof(float));
  if (rc != HB_MC_SUCCESS) return rc;

  hb_mc_dimension_t tg_dim = {.x = 8, .y = 4};
  hb_mc_dimension_t grid_dim = {.x = 1, .y = 1};
  uint32_t cuda_argv[4] = {(uint32_t)A_device, (uint32_t)B_device, (uint32_t)C_device, (uint32_t)n};

  rc = hb_mc_kernel_enqueue(&device, grid_dim, tg_dim,
                            "kernel_matmul_ab_dmem", 4, cuda_argv);
  if (rc != HB_MC_SUCCESS) return rc;

  rc = hb_mc_device_tile_groups_execute(&device);
  if (rc != HB_MC_SUCCESS) return rc;

  float C_host[MAT_N * MAT_N];
  rc = hb_mc_device_memcpy(&device, C_host,
                           (void *)(intptr_t)C_device,
                           n*n*sizeof(float), HB_MC_MEMCPY_TO_HOST);
  if (rc != HB_MC_SUCCESS) return rc;

  rc = hb_mc_device_finish(&device);
  if (rc != HB_MC_SUCCESS) return rc;

  float C_expected[MAT_N * MAT_N];
  host_matmul_naive(A_host, B_host, C_expected, n);

  float max_ferror = 0.0f;
  int mismatch = 0;
  for (int i = 0; i < n*n; i++) {
    float ferror = hb_mc_calculate_float_error(C_expected[i], C_host[i]);
    if (ferror > max_ferror) max_ferror = ferror;
    if (ferror > MAX_FLOAT_ERROR_TOLERANCE) {
      bsg_pr_err(BSG_RED("Mismatch") " C[%d]: got %.6f expected %.6f (err %.2e)\n",
                 i, C_host[i], C_expected[i], ferror);
      mismatch = 1;
    }
  }

  bsg_pr_test_info("Max relative FP error: %e\n", max_ferror);
  return mismatch ? HB_MC_FAIL : HB_MC_SUCCESS;
}

declare_program_main("gemm", kernel_gemm);
