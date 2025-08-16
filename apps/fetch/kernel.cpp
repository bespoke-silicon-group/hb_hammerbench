#include <bsg_manycore.h>
#include <bsg_tile_config_vars.h>
#include <bsg_cuda_lite_barrier.h>

int *A = (int *) 0x80000000;

extern "C" int kernel()
{
    register int *p = &A[(__bsg_x + bsg_tiles_X*(__bsg_y == bsg_tiles_Y-1 ? 1 : 0)) * BSG_CACHE_LINE_WORDS];
    bsg_barrier_tile_group_init();
    bsg_barrier_tile_group_sync();    
    bsg_cuda_print_stat_kernel_start();    
    if (__bsg_y == 0 || __bsg_y == bsg_tiles_Y-1) {
        for (int i = 0; i < N/(bsg_tiles_X*2); i++) {
            asm volatile ("lw x0, %0" :: "m"(*p) : "memory");        
            p += bsg_tiles_X * 2 * BSG_CACHE_LINE_WORDS;
        }
    }
    bsg_fence();
    bsg_cuda_print_stat_kernel_end();    
    bsg_barrier_tile_group_sync();
    return 0;
}
