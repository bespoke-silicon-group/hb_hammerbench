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
      mat1[i] = (float) (i % 7);
      mat2[i] = (float) (i % 3);
    } else if (PATTERN == 1) {
      uint32_t s1 = (uint32_t)(1103515245u * (uint32_t)(i + 12345) + 12345u);
      uint32_t s2 = (uint32_t)(1664525u   * (uint32_t)(i + 54321) + 1013904223u);
      mat1[i] = ((float)(s1 & 0xFFFFu) / 32768.0f) - 1.0f;
      mat2[i] = ((float)(s2 & 0xFFFFu) / 32768.0f) - 1.0f;
    } else {
      mat1[i] = ((i % 8) == 0) ? (float)((i % 13) - 6) : 0.0f;
      mat2[i] = ((i % 8) == 0) ? (float)((i % 5)  - 2) : 0.0f;
    }
  }
}

void host_mm(float *result, float *mat1, float *mat2, int n)
{
  for (int y = 0; y < n; y++) {
    for (int x = 0; x < n; x++) {
      float sum = 0.0f;
      for (int z = 0; z < n; z++) {
        sum += mat1[(n*y)+z]*mat2[(n*z)+x];
      }
      result[(n*y)+x] = sum;
    }
  }
}

int matmul_hb_parallel(int argc, char **argv)
{
  const char *bin_path = argv[1];
  printf("matmul_hb_parallel: N=%d NITER=%d PATTERN=%d\n", N, NITER, PATTERN);

  size_t total = (size_t)N * N;
  float *mat1 = (float*) malloc(NITER*total*sizeof(float));
  float *mat2 = (float*) malloc(NITER*total*sizeof(float));
  float *mat_result = (float*) malloc(NITER*total*sizeof(float));
  init_inputs(mat1, mat2, NITER*total);
  for (int i = 0; i < NITER; i++) {
    host_mm(&mat_result[total*i], &mat1[total*i], &mat2[total*i], N);
  }

  hb_mc_device_t device;
  BSG_CUDA_CALL(hb_mc_device_init(&device, "matmul_hb_parallel", HB_MC_DEVICE_ID));
  eva_t d_mat1, d_mat2, d_result;
  hb_mc_pod_id_t pod;
  hb_mc_device_foreach_pod_id(&device, pod) {
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));
    BSG_CUDA_CALL(hb_mc_device_program_init(&device, bin_path, ALLOC_NAME, 0));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, NITER*total*sizeof(float), &d_mat1));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, NITER*total*sizeof(float), &d_mat2));
    BSG_CUDA_CALL(hb_mc_device_malloc(&device, NITER*total*sizeof(float), &d_result));
    std::vector<hb_mc_dma_htod_t> htod_job;
    htod_job.push_back({d_mat1, mat1, NITER*total*sizeof(float)});
    htod_job.push_back({d_mat2, mat2, NITER*total*sizeof(float)});
    BSG_CUDA_CALL(hb_mc_device_transfer_data_to_device(&device, htod_job.data(), htod_job.size()));

    hb_mc_dimension_t tg_dim = { .x = bsg_tiles_X, .y = bsg_tiles_Y };
    hb_mc_dimension_t grid_dim = { .x = 1, .y = 1 };
    #define CUDA_ARGC 4
    uint32_t cuda_argv[CUDA_ARGC] = {d_mat1, d_mat2, d_result, pod};
    BSG_CUDA_CALL(hb_mc_kernel_enqueue(&device, grid_dim, tg_dim, "kernel", CUDA_ARGC, cuda_argv));
  }
  BSG_CUDA_CALL(hb_mc_device_pods_kernels_execute(&device));

  bool fail = false;
  float *actual_result = (float*) malloc(NITER*total*sizeof(float));
  hb_mc_device_foreach_pod_id(&device, pod) {
    BSG_CUDA_CALL(hb_mc_device_set_default_pod(&device, pod));
    std::vector<hb_mc_dma_dtoh_t> dtoh_job;
    dtoh_job.push_back({d_result, actual_result, NITER*total*sizeof(float)});
    BSG_CUDA_CALL(hb_mc_device_transfer_data_to_host(&device, dtoh_job.data(), dtoh_job.size()));
    float sse = 0.0f;
    for (int i = 0; i < NITER*total; i++) {
      double diff = (double)mat_result[i] - (double)actual_result[i];
      sse += diff*diff;
    }
    if (sse >= .01f) {
      printf("Matrix mismatch SSE=%f\n", sse);
      fail = true;
    }
  }

  free(mat1); free(mat2); free(mat_result); free(actual_result);
  BSG_CUDA_CALL(hb_mc_device_finish(&device));
  return fail ? HB_MC_FAIL : HB_MC_SUCCESS;
}

#include <bsg_manycore.h> // provide declare_program_main

// serial kernel included for completeness if one tile.
extern "C" int kernel(float *, float *, float *, int);
declare_program_main("matmul_hb_parallel", matmul_hb_parallel);
