#include "bsg_manycore.h"
#include "bsg_manycore.hpp"
#include "bsg_cuda_lite_barrier.h"
#include "global_pointer/global_pointer.hpp"
#include "util/lock.hpp"
#include "util/test_eq.hpp"
#include "util/statics.hpp"

DMEM(int) pod_x;
DMEM(int) pod_y;
DMEM(int) pod_id;

#define N 1

namespace lock_vars
{
static DRAM(util::lock) l;
static DRAM(int) x = 0, y = 0;
}

template <>
class bsg_global_pointer::reference<util::lock> {
    BSG_GLOBAL_POINTER_REFERENCE_CONSTRUCTORS(util::lock);
    BSG_GLOBAL_POINTER_REFERENCE_INTERNAL(util::lock);
    BSG_GLOBAL_POINTER_REFERENCE_METHOD(util::lock, acquire);
    BSG_GLOBAL_POINTER_REFERENCE_METHOD(util::lock, release);
};

extern "C" int lock()
{
    bsg_barrier_tile_group_init();
    bsg_barrier_tile_group_sync();

    using namespace bsg_global_pointer;
    reference<util::lock> l(&lock_vars::l);
    reference<int> x(&lock_vars::x);
    reference<int> y(&lock_vars::y);
    l.set_pod_x(0).set_pod_y(0);
    x.set_pod_x(0).set_pod_y(0);
    y.set_pod_x(0).set_pod_y(0);
    static constexpr int n = N;
    for (int i = 0; i < n; i++) {
        y =  y + 1;
    }
    bsg_barrier_tile_group_sync();

    for (int i = 0; i < n; i++) {
        l.acquire();
        x = x + 1;
        l.release();
    }
    bsg_barrier_tile_group_sync();
    bsg_barrier_tile_group_sync();
    return 0;
}

extern "C" int lock_check()
{
    using namespace lock_vars;
    bsg_barrier_tile_group_init();
    bsg_barrier_tile_group_sync();

    if (__bsg_id == 0 && pod_id == 0)
    {
        TEST_EQ(INT, x, N * bsg_tiles_X * bsg_tiles_Y * bsg_pods_X * bsg_pods_Y);  // assert lock works
        TEST_NEQ(INT, y, N * bsg_tiles_X * bsg_tiles_Y * bsg_pods_X * bsg_pods_Y); // assert lock doesn't work just by accident
    }
    bsg_barrier_tile_group_sync();
    return 0;
}

namespace tile_lock_vars
{
static DMEM(util::tile_lock) l;
static DRAM(int) x = 0, y = 0;
}

extern "C" int tile_lock()
{
    bsg_barrier_tile_group_init();
    bsg_barrier_tile_group_sync();

    using namespace bsg_global_pointer;
    reference<util::lock> l(bsg_tile_group_remote_pointer<util::tile_lock>(0,0,&tile_lock_vars::l));
    reference<int> x(&tile_lock_vars::x);
    reference<int> y(&tile_lock_vars::y);
    l.set_pod_x(0).set_pod_y(0);
    x.set_pod_x(0).set_pod_y(0);
    y.set_pod_x(0).set_pod_y(0);

    static constexpr int n = N;
    for (int i = 0; i < n; i++) {
        y = y + 1;
    }
    bsg_barrier_tile_group_sync();

    for (int i = 0; i < n; i++) {
        l.acquire();
        x = x + 1;
        l.release();
    }
    bsg_barrier_tile_group_sync();
    return 0;
}

extern "C" int tile_lock_check()
{
    using namespace tile_lock_vars;
    bsg_barrier_tile_group_init();
    bsg_barrier_tile_group_sync();

    if (__bsg_id == 0 && pod_id == 0)
    {
        TEST_EQ(INT, x, N * bsg_tiles_X * bsg_tiles_Y * bsg_pods_X * bsg_pods_Y);  // assert lock works
        TEST_NEQ(INT, y, N * bsg_tiles_X * bsg_tiles_Y * bsg_pods_X * bsg_pods_Y); // assert lock doesn't work just by accident
    }
    bsg_barrier_tile_group_sync();
    return 0;
}


extern "C" int init(int pod_x_, int pod_y_)
{
    pod_x = pod_x_;
    pod_y = pod_y_;
    pod_id = pod_x + pod_y * bsg_pods_X;
    bsg_barrier_tile_group_init();
    bsg_barrier_tile_group_sync();
    return 0;
}
