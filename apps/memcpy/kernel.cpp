#include <bsg_manycore.h>
#include <bsg_cuda_lite_barrier.h>

extern "C" __attribute__ ((noinline))
int
kernel_memcpy(int * A, int * B, int N) {

  bsg_barrier_hw_tile_group_init();
  bsg_cuda_print_stat_kernel_start();

  bsg_fence();
  // Credit Limit
  //int crlim = 16;
  //asm volatile ("csrw 0xfc0, %[crlim]" : : [crlim] "r" (crlim));

  int len = N/(bsg_tiles_X*bsg_tiles_Y);
  int *myA = (int*) &A[len*__bsg_id];
  int *myB = (int*) &B[len*__bsg_id];

  for (int i = 0; i < len; i += 16) {
    register int tmp00 = myA[i+0];
    register int tmp01 = myA[i+1];
    register int tmp02 = myA[i+2];
    register int tmp03 = myA[i+3];
    register int tmp04 = myA[i+4];
    register int tmp05 = myA[i+5];
    register int tmp06 = myA[i+6];
    register int tmp07 = myA[i+7];
    register int tmp08 = myA[i+8];
    register int tmp09 = myA[i+9];
    register int tmp10 = myA[i+10];
    register int tmp11 = myA[i+11];
    register int tmp12 = myA[i+12];
    register int tmp13 = myA[i+13];
    register int tmp14 = myA[i+14];
    register int tmp15 = myA[i+15];
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
