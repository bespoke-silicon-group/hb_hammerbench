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

// simple block-based parallel matmul
// each tile computes one BLOCK_DIM x BLOCK_DIM submatrix of C

#define BLOCK_DIM 16

extern "C"
int kernel(float *mat1, float *mat2, float *result, int pod_id)
{
  int tidx = __bsg_x;
  int tidy = __bsg_y;
  int n_tiles_x = bsg_tiles_X;
  int n_tiles_y = bsg_tiles_Y;

  // one iteration only for simplicity
  for (int iter = 0; iter < NITER; ++iter) {
    float *A = &mat1[N*N*iter];
    float *B = &mat2[N*N*iter];
    float *C = &result[N*N*iter];

    // each tile reports when it starts and finishes its block work
#ifdef WARM_CACHE
    warmup(A, B, N * N);
    bsg_barrier_tile_group_sync();
#endif
    bsg_cuda_print_stat_kernel_start();

    int n = N;
    int blocks_per_row = (n + BLOCK_DIM - 1) / BLOCK_DIM;
    int by = tidy;
    while (by < blocks_per_row) {
      int bx = tidx;
      while (bx < blocks_per_row) {
        float localC[BLOCK_DIM*BLOCK_DIM];
        // zero local output
        for (int i = 0; i < BLOCK_DIM*BLOCK_DIM; i++)
          localC[i] = 0.0f;

        // iterate over k dimension blocks
        for (int kz = 0; kz < blocks_per_row; ++kz) {
          float localA[BLOCK_DIM*BLOCK_DIM];
          float localB[BLOCK_DIM*BLOCK_DIM];
          // load A block
          int a_row = by * BLOCK_DIM;
          int a_col = kz * BLOCK_DIM;
          for (int y = 0; y < BLOCK_DIM; ++y) {
            for (int x = 0; x < BLOCK_DIM; ++x) {
              int src_y = a_row + y;
              int src_x = a_col + x;
              if (src_y < n && src_x < n)
                localA[y*BLOCK_DIM + x] = A[src_y*n + src_x];
              else
                localA[y*BLOCK_DIM + x] = 0.0f;
            }
          }
          // load B block
          int b_row = kz * BLOCK_DIM;
          int b_col = bx * BLOCK_DIM;
          for (int y = 0; y < BLOCK_DIM; ++y) {
            for (int x = 0; x < BLOCK_DIM; ++x) {
              int src_y = b_row + y;
              int src_x = b_col + x;
              if (src_y < n && src_x < n)
                localB[y*BLOCK_DIM + x] = B[src_y*n + src_x];
              else
                localB[y*BLOCK_DIM + x] = 0.0f;
            }
          }
          // multiply-accumulate (register blocked 1x4)
          for (int y = 0; y < BLOCK_DIM; ++y) {
            int x = 0;

            for (; x <= BLOCK_DIM - 4; x += 4) {
              float sum0 = localC[y*BLOCK_DIM + x];
              float sum1 = localC[y*BLOCK_DIM + x + 1];
              float sum2 = localC[y*BLOCK_DIM + x + 2];
              float sum3 = localC[y*BLOCK_DIM + x + 3];

              for (int k = 0; k < BLOCK_DIM; ++k) {
                float a = localA[y*BLOCK_DIM + k];
                int b_base = k*BLOCK_DIM + x;
                sum0 += a * localB[b_base];
                sum1 += a * localB[b_base + 1];
                sum2 += a * localB[b_base + 2];
                sum3 += a * localB[b_base + 3];
              }

              localC[y*BLOCK_DIM + x] = sum0;
              localC[y*BLOCK_DIM + x + 1] = sum1;
              localC[y*BLOCK_DIM + x + 2] = sum2;
              localC[y*BLOCK_DIM + x + 3] = sum3;
            }

            for (; x < BLOCK_DIM; ++x) {
              float sum = localC[y*BLOCK_DIM + x];
              for (int k = 0; k < BLOCK_DIM; ++k) {
                sum += localA[y*BLOCK_DIM + k] * localB[k*BLOCK_DIM + x];
              }
              localC[y*BLOCK_DIM + x] = sum;
            }
          }
        }
        // write back localC to global C
        int c_row = by * BLOCK_DIM;
        int c_col = bx * BLOCK_DIM;
        for (int y = 0; y < BLOCK_DIM; ++y) {
          for (int x = 0; x < BLOCK_DIM; ++x) {
            int dst_y = c_row + y;
            int dst_x = c_col + x;
            if (dst_y < n && dst_x < n) {
              C[dst_y*n + dst_x] = localC[y*BLOCK_DIM + x];
            }
          }
        }
        bx += n_tiles_x;
      }
      by += n_tiles_y;
    }

    bsg_cuda_print_stat_kernel_end();
  }
  bsg_fence();
  bsg_barrier_tile_group_sync();
  return 0;
}
