#include <cello/cello_start.hpp>
#include <cello/cello_main.hpp>
#include <bsg_manycore.h>
#include <bsg_cuda_lite_barrier.h>

int cello_start()
{
    bsg_barrier_tile_group_init();
    bsg_barrier_tile_group_sync();
    bsg_printf("hello from the start function\n");
    if (__bsg_id == 0) {
        cello_main(0, nullptr);
    }
    bsg_barrier_tile_group_sync();
    return 0;
}
