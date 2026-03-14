#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <bsg_manycore_cuda.h>
#include <bsg_manycore_regression.h>

#define ALLOC_NAME "default_allocator"

#ifndef PATTERN
#define PATTERN 0
#endif

static inline void init_inputs(float *mat1, float *mat2, int total)
{
  for (int i = 0; i < total; i++) {
    if (PATTERN == 0) {
      // baseline deterministic modulo pattern
      mat1[i] = (float) (i % 7);
      mat2[i] = (float) (i % 3);
    } else if (PATTERN == 1) {
      // deterministic pseudo-random pattern (bounded to [-1, 1])
      uint32_t s1 = (uint32_t)(1103515245u * (uint32_t)(i + 12345) + 12345u);
      uint32_t s2 = (uint32_t)(1664525u * (uint32_t)(i + 54321) + 1013904223u);
      mat1[i] = ((float)(s1 & 0xFFFFu) / 32768.0f) - 1.0f;
      mat2[i] = ((float)(s2 & 0xFFFFu) / 32768.0f) - 1.0f;
    } else {
      // sparse-ish deterministic pattern
      mat1[i] = ((i % 8) == 0) ? (float)((i % 13) - 6) : 0.0f;
      mat2[i] = ((i % 8) == 0) ? (float)((i % 5) - 2) : 0.0f;
    }
  }
}

void host_mm(float *result, float *mat1, float *mat2)
{
  for (int y = 0; y < N; y++) {
    for (int x = 0; x < N; x++) {
      float sum = 0.0f;
      for (int z = 0; z < N; z++) {
        sum += mat1[(N*y)+z]*mat2[(N*z)+x];
      }
      result[(N*y)+x] = sum;
    }
  }
}

// Host Main;
int matmul_hb_serial(int argc, char **argv)
{
  const char *bin_path = argv[1];

  printf("matmul_hb_serial: N=%d NITER=%d PATTERN=%d\n", N, NITER, PATTERN);

  float *mat1 = (float*) malloc(NITER*N*N*sizeof(float));
  float *mat2 = (float*) malloc(NITER*N*N*sizeof(float));
  float *mat_result = (float*) malloc(NITER*N*N*sizeof(float));

  init_inputs(mat1, mat2, NITER*N*N);

  for (int i = 0; i < NITER; i++) {
    host_mm(&mat_result[N*N*i], &mat1[N*N*i], &mat2[N*N*i]);
  }

  hb_mc_device_t device;
  BSG_CUDA_CALL(hb_mc_device_init(&device, "matmul_hb_serial", HB_MC_DEVICE_ID));

  eva_t d_mat1, d_mat2, d_result;

  hb_mc_pod_id_t pod;
  hb_mc_device_foreach_pod_id(&device, pod) {
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));
    BSG_CUDA_CALL(hb_mc_device_program_init(&device, bin_path, ALLOC_NAME, 0));

    BSG_CUDA_CALL(hb_mc_device_malloc(&device, NITER*N*N*sizeof(float), &d_mat1));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, NITER*N*N*sizeof(float), &d_mat2));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, NITER*N*N*sizeof(float), &d_result));

    // Transfer to device
    std::vector<hb_mc_dma_htod_t> htod_job;
    htod_job.push_back({d_mat1, mat1, NITER*N*N*sizeof(float)});
    htod_job.push_back({d_mat2, mat2, NITER*N*N*sizeof(float)});
    BSG_CUDA_CALL(hb_mc_device_transfer_data_to_device(&device, htod_job.data(), htod_job.size()));

    // Launch single-tile kernel (serial on one core)
    hb_mc_dimension_t tg_dim = { .x = 1, .y = 1 };
    hb_mc_dimension_t grid_dim = { .x = 1, .y = 1 };
    #define CUDA_ARGC 4
    uint32_t cuda_argv[CUDA_ARGC] = {d_mat1, d_mat2, d_result, pod};

    BSG_CUDA_CALL(hb_mc_kernel_enqueue(&device, grid_dim, tg_dim, "kernel", CUDA_ARGC, cuda_argv));
  }

  // Execute kernels
  BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&device));

  // Read back and validate
  bool fail = false;
  float *actual_result = (float*) malloc(NITER*N*N*sizeof(float));

  hb_mc_device_foreach_pod_id(&device, pod) {
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));
    std::vector<hb_mc_dma_dtoh_t> dtoh_job;
    dtoh_job.push_back({d_result, actual_result, NITER*N*N*sizeof(float)});
    BSG_CUDA_CALL(hb_mc_device_transfer_data_to_host(&device, dtoh_job.data(), dtoh_job.size()));

    // validate
    float sse = 0.0f;
    for (int i = 0; i < NITER*N*N; i++) {
      float actual = actual_result[i];
      float expected = mat_result[i];
      float diff = expected - actual;
      sse += diff*diff;
    }
    if (sse >= .01f) {
      printf("Matrix Mismatch. SSE= %f\n", sse);
      fail = true;
    }
  }

  free(mat1);
  free(mat2);
  free(mat_result);
  free(actual_result);

  BSG_CUDA_CALL(hb_mc_device_finish(&device));
  return fail ? HB_MC_FAIL : HB_MC_SUCCESS;
}

declare_program_main("matmul_hb_serial", matmul_hb_serial);
