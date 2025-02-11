#include "global_pointer/global_pointer.hpp"
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_cuda_lite_barrier.h"


using bsg_global_pointer::pod_address;
using bsg_global_pointer::pod_address_guard;

__attribute__((section(".dram"))) unsigned g_pod_x = 0;
__attribute__((section(".dram"))) unsigned g_pod_y = 0;

extern "C" int setup(unsigned pod_x, unsigned pod_y)
{
        bsg_barrier_tile_group_init();
        bsg_barrier_tile_group_sync();
        if (__bsg_id == 0) {
                g_pod_x = pod_x;
                g_pod_y = pod_y;
                bsg_print_unsigned(g_pod_x);
                bsg_print_unsigned(g_pod_y);
        }
        bsg_barrier_tile_group_sync();
        return 0;
}

extern "C" int multipod_pointer(unsigned pod_x, unsigned pod_y)
{
        bsg_barrier_tile_group_init();
        bsg_barrier_tile_group_sync();
        if (__bsg_id == 0) {
                pod_address addr{0};
                addr.set_pod_x(0).set_pod_y(0);
                pod_address_guard grd(addr);
                bsg_print_unsigned(g_pod_x);
                bsg_print_unsigned(g_pod_y);
        }
        bsg_barrier_tile_group_sync();
        return 0;
}
