#include <bsg_manycore.h>
#include <bsg_manycore.hpp>
#include <bsg_cuda_lite_barrier.h>
#ifndef LOADS
#error "Define LOADS"
#endif

constexpr unsigned clog2(unsigned value)
{
    unsigned exp = 0;
    while (1<<exp < value) {
        exp++;
    }
    return exp;
}

int group_mem [LOADS];

extern "C" int group_read()
{
    for (int i = 0; i < LOADS; i++) {
        group_mem[i] = __bsg_x + bsg_tiles_X*__bsg_y + (bsg_tiles_X*bsg_tiles_Y)*i;
    }
    bsg_barrier_hw_tile_group_sync();
    bsg_cuda_print_stat_kernel_start();
    if (__bsg_id == 0) {
        int sum = 0;
        for (unsigned i = 0; i < LOADS; i++) {
            unsigned j = (i >> clog2(bsg_tiles_X*bsg_tiles_Y));
            unsigned y = (i >> clog2(bsg_tiles_X)) & (bsg_tiles_Y-1);
            unsigned x = i & (bsg_tiles_X-1);
            // bsg_print_int(x);
            // bsg_print_int(y);
            // bsg_print_int(j);            
            int *p = bsg_tile_group_remote_pointer(x, y, &group_mem[j]);
            bsg_print_int(*p);
            sum += *p;
        }
    }
    bsg_barrier_hw_tile_group_sync();
    bsg_cuda_print_stat_kernel_end();    
    bsg_fence();
    bsg_barrier_hw_tile_group_sync();
    return 0;
}
