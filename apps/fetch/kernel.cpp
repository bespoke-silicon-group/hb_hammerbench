#include <bsg_manycore.h>
#include <bsg_tile_config_vars.h>
#include <bsg_cuda_lite_barrier.h>

__attribute__((section(".dram"))) int *A;

#define WORDS (SIZE/4)
#define LINES (WORDS/BSG_CACHE_LINE_WORDS)

extern "C" int kernel()
{
    register int *p_start = &A[(__bsg_x + bsg_tiles_X*(__bsg_y == bsg_tiles_Y-1 ? 1 : 0)) * BSG_CACHE_LINE_WORDS];
    register int *p = p_start;
    bsg_barrier_tile_group_init();
    bsg_barrier_tile_group_sync();    
    bsg_cuda_print_stat_kernel_start();    
    if (__bsg_y == 0 || __bsg_y == bsg_tiles_Y-1) {
        for (int i = 0, j = 0; i < N; ) {
            asm volatile ("lw x0, %0" :: "m"(*p) : "memory");
            i += bsg_tiles_X*2;
            j += bsg_tiles_X*2;
            if (j > LINES) {
                p = p_start;
                j = 0;
            } else {
                p += bsg_tiles_X*2*BSG_CACHE_LINE_WORDS;
            }
        }
    }
    bsg_fence();
    bsg_cuda_print_stat_kernel_end();    
    bsg_barrier_tile_group_sync();
    return 0;
}
