#include <bsg_manycore.h>
#include <bsg_cuda_lite_barrier.h>

#ifdef WARM_CACHE
#ifndef CACHE_LINE_WORDS
#error "CACHE_LINE_WORDS not defined"
#endif
__attribute__((noinline))
static void warmup(float *A, float *B, int N)
{
  for (int i = __bsg_id*CACHE_LINE_WORDS; i < N; i += bsg_tiles_X*bsg_tiles_Y*CACHE_LINE_WORDS) {
      asm volatile ("lw x0, %[p]" :: [p] "m" (A[i]));
      asm volatile ("lw x0, %[p]" :: [p] "m" (B[i]));
  }
  bsg_fence();
}
#endif

extern "C" __attribute__ ((noinline))
int
kernel_memcpy(float * A, float * B, int N) {

  bsg_barrier_hw_tile_group_init();
#ifdef WARM_CACHE
  warmup(A, B, N);
#endif
  bsg_barrier_hw_tile_group_sync();
  bsg_cuda_print_stat_kernel_start();

  #define STRIDE (bsg_tiles_X*bsg_tiles_Y*STRIPE*NUM_ITER)
  bsg_unroll(1)
  for (int i = 0; i < NUM_ITER; i++) {
    int start = ((i*bsg_tiles_X*bsg_tiles_Y)+__bsg_id)*STRIPE;
    float * currA = &A[start];
    float * currB = &B[start];
    bsg_unroll(1)
    for (int r = 0; r < DEPTH; r++) {
      register float temp[STRIPE];
      bsg_unroll(STRIPE)
      for (int j = 0; j < STRIPE; j++) {
        temp[j] = currA[j];
      }
      asm volatile("": : :"memory");

      bsg_unroll(STRIPE)
      for (int j = 0; j < STRIPE; j++) {
        currB[j] = temp[j];
      }
      asm volatile("": : :"memory");
      currA += STRIDE;
      currB += STRIDE;
    }
  }

  bsg_fence();
  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();

  return 0;
}
