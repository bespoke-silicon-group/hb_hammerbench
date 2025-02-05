#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_cuda_lite_barrier.h"
extern "C" int multipod_pointer()
{
        bsg_barrier_tile_group_init();
        bsg_barrier_tile_group_sync();
        bsg_print_int(__bsg_id);
        bsg_barrier_tile_group_sync();
        return 0;
}
