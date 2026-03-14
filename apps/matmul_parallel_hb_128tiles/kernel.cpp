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

// Fair-ish parallel matmul with direct A/B accesses.
// For N values that divide the tile count, rows are split into enough
// contiguous column chunks to create one useful task per tile.

extern "C"
int kernel(float *mat1, float *mat2, float *result, int pod_id)
{
  int tile_id = __bsg_id;
  int n_tiles = bsg_tiles_X * bsg_tiles_Y;

  bsg_barrier_tile_group_init();
  bsg_barrier_tile_group_sync();

  for (int iter = 0; iter < NITER; ++iter) {
    float *A = &mat1[N*N*iter];
    float *B = &mat2[N*N*iter];
    float *C = &result[N*N*iter];

#ifdef WARM_CACHE
    warmup(A, B, N * N);
    bsg_barrier_tile_group_sync();
#endif
    bsg_cuda_print_stat_kernel_start();

    int chunks_per_row = 1;
    if ((N <= n_tiles) && ((n_tiles % N) == 0)) {
      chunks_per_row = n_tiles / N;
    }
    int chunk_cols = (N + chunks_per_row - 1) / chunks_per_row;
    int total_tasks = N * chunks_per_row;

    for (int task_id = tile_id; task_id < total_tasks; task_id += n_tiles) {
      int y = task_id / chunks_per_row;
      int chunk = task_id % chunks_per_row;
      int x_start = chunk * chunk_cols;
      int x_end = x_start + chunk_cols;
      if (x_end > N) {
        x_end = N;
      }

      int x = x_start;
      for (; x <= x_end - 4; x += 4) {
        float sum0 = 0.0f;
        float sum1 = 0.0f;
        float sum2 = 0.0f;
        float sum3 = 0.0f;

        for (int z = 0; z < N; ++z) {
          float a = A[(N * y) + z];
          int b_base = (N * z) + x;
          sum0 += a * B[b_base];
          sum1 += a * B[b_base + 1];
          sum2 += a * B[b_base + 2];
          sum3 += a * B[b_base + 3];
        }

        int c_base = (N * y) + x;
        C[c_base] = sum0;
        C[c_base + 1] = sum1;
        C[c_base + 2] = sum2;
        C[c_base + 3] = sum3;
      }

      for (; x < x_end; ++x) {
        float sum = 0.0f;
        for (int z = 0; z < N; ++z) {
          sum += A[(N * y) + z] * B[(N * z) + x];
        }
        C[(N * y) + x] = sum;
      }
    }

    bsg_cuda_print_stat_kernel_end();
  }
  bsg_fence();
  bsg_barrier_tile_group_sync();
  return 0;
}
