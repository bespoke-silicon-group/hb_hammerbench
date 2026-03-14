#include <bsg_manycore.h>
#include <bsg_cuda_lite_barrier.h>

#ifdef WARM_CACHE
#ifndef CACHE_LINE_WORDS
#error "CACHE_LINE_WORDS not defined"
#endif
__attribute__((noinline))
static void warmup(float *A, float *B, int total)
{
  for (int i = __bsg_id * CACHE_LINE_WORDS;
       i < total;
       i += bsg_tiles_X * bsg_tiles_Y * CACHE_LINE_WORDS) {
    asm volatile ("lw x0, %[p]" :: [p] "m" (A[i]));
    asm volatile ("lw x0, %[p]" :: [p] "m" (B[i]));
  }
  bsg_fence();
}
#endif

extern "C"
int kernel(float *mat1, float *mat2, float *result, int pod_id)
{
  // Single-tile serial kernel: compute whole matrix on one core
  bsg_barrier_tile_group_init();
  bsg_barrier_tile_group_sync();

  for (int iter = 0; iter < NITER; iter++) {
    float *curr_mat1 = &mat1[N*N*iter];
    float *curr_mat2 = &mat2[N*N*iter];
    float *curr_result = &result[N*N*iter];

#ifdef WARM_CACHE
    warmup(curr_mat1, curr_mat2, N * N);
#endif

    // record when this tile begins work on the matrix
    bsg_cuda_print_stat_kernel_start();

    for (int y = 0; y < N; y++) {
      int x = 0;

      for (; x <= N - 4; x += 4) {
        float sum0 = 0.0f;
        float sum1 = 0.0f;
        float sum2 = 0.0f;
        float sum3 = 0.0f;

        for (int z = 0; z < N; z++) {
          float a = curr_mat1[(N * y) + z];
          int mat2_base = (N * z) + x;

          sum0 += a * curr_mat2[mat2_base];
          sum1 += a * curr_mat2[mat2_base + 1];
          sum2 += a * curr_mat2[mat2_base + 2];
          sum3 += a * curr_mat2[mat2_base + 3];
        }

        int result_base = (N * y) + x;
        curr_result[result_base] = sum0;
        curr_result[result_base + 1] = sum1;
        curr_result[result_base + 2] = sum2;
        curr_result[result_base + 3] = sum3;
      }

      for (; x < N; x++) {
        float sum = 0.0f;
        for (int z = 0; z < N; z++) {
          sum += curr_mat1[(N * y) + z] * curr_mat2[(N * z) + x];
        }
        curr_result[(N * y) + x] = sum;
      }
    }

    // done with this iteration
    bsg_cuda_print_stat_kernel_end();
  }

  bsg_fence();
  bsg_barrier_tile_group_sync();
  return 0;
}
