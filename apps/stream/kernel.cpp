#include <standard/standard.hpp>
#include <util/statics.hpp>
#include <bsg_manycore.h>
#include <bsg_cuda_lite_barrier.h>
DRAM(int) *A, *B;
DRAM(int)   S;

int kernel()
{
    int *A_l, *B_l;
    A_l = A;
    B_l = B;
    bsg_fence();
    bsg_barrier_tile_group_init();
    bsg_barrier_tile_group_sync();
    bsg_cuda_print_stat_kernel_start();
    register int sum = 0;
    for (int i = __bsg_id*16; i < SIZE; i += bsg_tiles_X*bsg_tiles_Y*16) {
        register int tmp00 = 0;
        register int tmp01 = 0;
        register int tmp02 = 0;
        register int tmp03 = 0;
        register int tmp04 = 0;
        register int tmp05 = 0;
        register int tmp06 = 0;
        register int tmp07 = 0;
        register int tmp08 = 0;
        register int tmp09 = 0;
        register int tmp10 = 0;
        register int tmp11 = 0;
        register int tmp12 = 0;
        register int tmp13 = 0;
        register int tmp14 = 0;
        register int tmp15 = 0;
#if defined(READ) || defined(COPY)
        tmp00 = A_l[i+0];
        bsg_fence();  // this fence makes sure that next packets don't block the network.
        tmp01 = A_l[i+1];
        tmp02 = A_l[i+2];
        tmp03 = A_l[i+3];
        tmp04 = A_l[i+4];
        tmp05 = A_l[i+5];
        tmp06 = A_l[i+6];
        tmp07 = A_l[i+7];
        tmp08 = A_l[i+8];
        tmp09 = A_l[i+9];
        tmp10 = A_l[i+10];
        tmp11 = A_l[i+11];
        tmp12 = A_l[i+12];
        tmp13 = A_l[i+13];
        tmp14 = A_l[i+14];
        tmp15 = A_l[i+15];
#endif
        asm volatile("": : :"memory");
#if defined(READ)
        sum += tmp00+tmp01+tmp02+tmp03+tmp04+tmp05+tmp06;
        sum += tmp07+tmp08+tmp09+tmp10+tmp11+tmp12+tmp13;
        sum += tmp14+tmp15;
#endif

#if defined(WRITE) || defined(COPY)
        B_l[i+0] = tmp00;
        bsg_fence();  // this fence makes sure that next packets don't block the network.
        B_l[i+1] = tmp01;
        B_l[i+2] = tmp02;
        B_l[i+3] = tmp03;
        B_l[i+4] = tmp04;
        B_l[i+5] = tmp05;
        B_l[i+6] = tmp06;
        B_l[i+7] = tmp07;
        B_l[i+8] = tmp08;
        B_l[i+9] = tmp09;
        B_l[i+10] = tmp10;
        B_l[i+11] = tmp11;
        B_l[i+12] = tmp12;
        B_l[i+13] = tmp13;
        B_l[i+14] = tmp14;
        B_l[i+15] = tmp15;
#endif
    }

    bsg_cuda_print_stat_kernel_end();
    bsg_barrier_tile_group_sync();
    return sum;
}
