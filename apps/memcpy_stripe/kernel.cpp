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


  for (int i  = __bsg_id*STRIPE; i < N; i += bsg_tiles_X*bsg_tiles_Y*STRIPE) {
    register float temp[STRIPE];
    bsg_unroll(STRIPE)
    for (int j = 0; j < STRIPE; j++) {
      temp[j] = A[i+j];
    }
    asm volatile("": : :"memory");

    bsg_unroll(STRIPE)
    for (int j = 0; j < STRIPE; j++) {
      B[i+j] = temp[j];
    }
    asm volatile("": : :"memory");
  }

  bsg_fence();
  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();

  return 0;
}
