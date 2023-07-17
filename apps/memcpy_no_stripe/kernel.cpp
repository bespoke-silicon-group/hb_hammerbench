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
  
  # define UNROLL 16
  int start = __bsg_id * num_words;
  float * myA = &A[start];
  float * myB = &B[start];
  for (int i = 0; i < num_words; i+=UNROLL) {
    register float tmp00 = myA[i+0];
    register float tmp01 = myA[i+1];
    register float tmp02 = myA[i+2];
    register float tmp03 = myA[i+3];
    register float tmp04 = myA[i+4];
    register float tmp05 = myA[i+5];
    register float tmp06 = myA[i+6];
    register float tmp07 = myA[i+7];
    register float tmp08 = myA[i+8];
    register float tmp09 = myA[i+9];
    register float tmp10 = myA[i+10];
    register float tmp11 = myA[i+11];
    register float tmp12 = myA[i+12];
    register float tmp13 = myA[i+13];
    register float tmp14 = myA[i+14];
    register float tmp15 = myA[i+15];
    asm volatile("": : :"memory");
    myB[i+0] = tmp00;
    myB[i+1] = tmp01;
    myB[i+2] = tmp02;
    myB[i+3] = tmp03;
    myB[i+4] = tmp04;
    myB[i+5] = tmp05;
    myB[i+6] = tmp06;
    myB[i+7] = tmp07;
    myB[i+8] = tmp08;
    myB[i+9] = tmp09;
    myB[i+10] = tmp10;
    myB[i+11] = tmp11;
    myB[i+12] = tmp12;
    myB[i+13] = tmp13;
    myB[i+14] = tmp14;
    myB[i+15] = tmp15;
  }

  bsg_fence();
  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();

  return 0;
}
