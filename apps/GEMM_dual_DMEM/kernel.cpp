#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_tile_group_barrier.hpp"

bsg_barrier<bsg_tiles_X, bsg_tiles_Y> barrier;

extern "C" __attribute__ ((noinline))
int kernel_matmul_ab_dmem(float *A, float *B, float *C, int N) {

  bsg_cuda_print_stat_kernel_start();

  int tile_id = __bsg_y * bsg_tiles_X + __bsg_x;
  int num_tiles = bsg_tiles_X * bsg_tiles_Y;

  // Dual-DMEM data staging for N=32, 32-core tg (8x4)
  float a_cache[1][32];
  float col_buf[32];

  int owned = 0;
  for (int i = tile_id; i < N; i += num_tiles) {
    for (int k = 0; k < N; k++)
      a_cache[owned][k] = A[i*N+k];
    owned++;
  }

  for (int j = 0; j < N; j++) {
    for (int k = 0; k < N; k++)
      col_buf[k] = B[k*N+j];

    int r = 0;
    for (int i = tile_id; i < N; i += num_tiles) {
      float sum = 0.0f;
      for (int k = 0; k < N; k++)
        sum += a_cache[r][k] * col_buf[k];
      C[i*N+j] = sum;
      r++;
    }
  }

  bsg_cuda_print_stat_kernel_end();
  barrier.sync();
  return 0;
}
