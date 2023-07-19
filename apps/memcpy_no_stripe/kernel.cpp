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
  int num_words = N / (bsg_tiles_X*bsg_tiles_Y);
  bsg_barrier_hw_tile_group_sync();
  bsg_cuda_print_stat_kernel_start();


  float *myA = &A[__bsg_id*WIDTH];
  float *myB = &B[__bsg_id*WIDTH];

  bsg_unroll(1)
  for (int i = 0; i < NUM_ITER; i++) {
    bsg_unroll(1)
    for (int j = 0; j < WIDTH; j+=CACHE_LINE_WORDS) {
      register float temp[CACHE_LINE_WORDS];  

      bsg_unroll(CACHE_LINE_WORDS)
      for (int t = 0; t < CACHE_LINE_WORDS; t++) {
        temp[t] = myA[j+t];
      }

      bsg_unroll(CACHE_LINE_WORDS)
      for (int t = 0; t < CACHE_LINE_WORDS; t++) {
        myB[j+t] = temp[t];
      }
    }
    myA += (WIDTH*bsg_tiles_X*bsg_tiles_Y);
    myB += (WIDTH*bsg_tiles_X*bsg_tiles_Y);
  }



  bsg_fence();
  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();

  return 0;
}
