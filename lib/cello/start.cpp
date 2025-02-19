#include <cello/cello.hpp>
#include <cello/scheduler.hpp>
#include <bsg_manycore.h>
#include <bsg_cuda_lite_barrier.h>

namespace  cello {
__attribute__((section(".dram")))
int main_complete = 0;
}

using namespace cello;

int cello_start()
{
    bsg_barrier_tile_group_init();
    bsg_barrier_tile_group_sync();
    if (__bsg_id == 0) {
        cello_main(0, nullptr);
        main_complete = 1;
    } else {
        scheduler_loop([](){ return main_complete; });
    }
    bsg_barrier_tile_group_sync();
    return 0;
}
