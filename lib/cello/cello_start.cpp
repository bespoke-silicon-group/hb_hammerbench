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
DRAM(int) main_complete = 0;
}

using namespace cello;

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
    bsg_global_pointer::reference<int> should_exit(&main_complete);
    should_exit.set_pod_x(0).set_pod_y(0);
    if (cello::my::id() == 0) {
        cello_main(0, nullptr);
        should_exit = 1;
    } else {
        scheduler_loop([=](){ return should_exit == 1; });
    }
    bsg_barrier_tile_group_sync();
    bsg_cuda_print_stat_kernel_end();
    bsg_barrier_tile_group_sync();
    return 0;
}
