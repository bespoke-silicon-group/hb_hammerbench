#include <cello/cello.hpp>
#include <cello/scheduler.hpp>
#include <cello/allocator.hpp>
#include <cello/thread_id.hpp>
#include <global_pointer/global_pointer.hpp>
#include <util/statics.hpp>
#include <bsg_manycore.h>
#include <bsg_cuda_lite_barrier.h>

namespace  cello
{
/**
 * set to 1 when main completes
 */
DMEM(int) should_exit;
}

using namespace cello;

/**
 * @brief propogate signal to  neighbors east and south neigbors
 */
static void signal_neighbors(int *local_var, int signal)
{
    thread_id_decoded decode;
    thread_id_decode(&decode, cello::my::id());

    thread_id_decoded east, south;
    east.pod_y = decode.pod_y;
    east.tile_y = decode.tile_y;
    if  (decode.tile_x == cello::my::num_tiles_x()-1) {
        east.pod_x = decode.pod_x + 1;
        east.tile_x = 0;
    } else {
        east.pod_x = decode.pod_x;
        east.tile_x = decode.tile_x + 1;
    }

    south.pod_x = decode.pod_x;
    south.tile_x = decode.tile_x;
    if (decode.tile_y == cello::my::num_tiles_y()-1) {
        south.pod_y = decode.pod_y + 1;
        south.tile_y = 0;
    } else {
        south.pod_y = decode.pod_y;
        south.tile_y = decode.tile_y + 1;
    }

    if (east.pod_x != cello::my::num_pods_x()) {
        // update east neighbor
        int *tg_ptr = bsg_tile_group_remote_pointer<int>(east.tile_x, east.tile_y, local_var);
        global_pointer<int> glb_ptr = global_pointer<int>::onPodXY(east.pod_x, east.pod_y, tg_ptr);
        *glb_ptr = signal;
    }

    if (south.pod_y != cello::my::num_pods_y()) {
        // update south neighbor
        int *tg_ptr = bsg_tile_group_remote_pointer<int>(south.tile_x, south.tile_y, local_var);
        global_pointer<int> glb_ptr = global_pointer<int>::onPodXY(south.pod_x, south.pod_y, tg_ptr);
        *glb_ptr = signal;
    }
}

int cello_setup(cello::config *config)
{
    bsg_barrier_tile_group_init();
    bsg_barrier_tile_group_sync();
    set_id_vars(config);
    allocator_initialize(config);
    bsg_barrier_tile_group_sync();
    scheduler_initialize(config);
    bsg_barrier_tile_group_sync();
    bsg_barrier_tile_group_sync();    
    return 0;
}

int cello_start(cello::config *config)
{
    bsg_barrier_tile_group_init();
    bsg_barrier_tile_group_sync();
    bsg_cuda_print_stat_kernel_start();
    if (cello::my::id() == 0) {
        cello_main(0, nullptr);
        should_exit = 1;
    } else {
        scheduler_loop([=](){
            return should_exit == 1;
        });
    }
    signal_neighbors(&should_exit, 1);
    bsg_barrier_tile_group_sync();
    bsg_cuda_print_stat_kernel_end();
    bsg_barrier_tile_group_sync();
    return 0;
}
