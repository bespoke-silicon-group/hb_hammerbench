#include "bsg_manycore.h"
#include "bsg_manycore.hpp"
#include "bsg_cuda_lite_barrier.h"
#include "util/lock.hpp"
#include "util/test_eq.hpp"
#include "util/statics.hpp"

#define N 1

namespace lock_vars
{
static DRAM(util::lock) l;
static DRAM(int) x = 0, y = 0;
}

int lock()
{
    using namespace lock_vars;
    static constexpr int n = N;
    for (int i = 0; i < n; i++) {
        y += 1;
    }
    bsg_barrier_tile_group_sync();

    for (int i = 0; i < n; i++) {
        l.acquire();
        x += 1;
        l.release();
    }
    bsg_barrier_tile_group_sync();

    if (__bsg_id == 0)
    {
        TEST_EQ(INT, x, n * bsg_tiles_X * bsg_tiles_Y);  // assert lock works
        TEST_NEQ(INT, y, n * bsg_tiles_X * bsg_tiles_Y); // assert lock doesn't work just by accident
    }
    bsg_barrier_tile_group_sync();
    return 0;
}

namespace tile_lock_vars
{
static DMEM(util::tile_lock) l;
static DRAM(int) x = 0, y = 0;
}

int tile_lock()
{
    using namespace tile_lock_vars;
    util::tile_lock *lp = bsg_tile_group_remote_pointer<util::tile_lock>(0, 0, &l);
    static constexpr int n = N;
    for (int i = 0; i < n; i++) {
        y += 1;
    }
    bsg_barrier_tile_group_sync();

    for (int i = 0; i < n; i++) {
        lp->acquire();
        x += 1;
        lp->release();
    }
    bsg_barrier_tile_group_sync();

    if (__bsg_id == 0)
    {
        TEST_EQ(INT, x, n * bsg_tiles_X * bsg_tiles_Y);  // assert lock works
        TEST_NEQ(INT, y, n * bsg_tiles_X * bsg_tiles_Y); // assert lock doesn't work just by accident
    }
    bsg_barrier_tile_group_sync();
    return 0;
}

namespace lock_guard_vars
{
static DRAM(util::lock) l;
static DRAM(int) x = 0;
}

int lock_guard()
{
    using namespace lock_guard_vars;
    bsg_barrier_tile_group_sync();
    static constexpr int n = N;
    for (int i = 0; i < n; i++) {
        util::lock_guard<util::lock> g(l);
        x += 1;
        bsg_fence();
    }

    bsg_barrier_tile_group_sync();
    if (__bsg_id == 0)
    {
        TEST_EQ(INT, x, n * bsg_tiles_X * bsg_tiles_Y);  // assert lock works
    }
    bsg_barrier_tile_group_sync();
    return 0;
}

namespace tile_lock_guard_vars
{
DMEM(util::tile_lock) l;
DRAM(int) x = 0;
}
int tile_lock_guard()
{
    using namespace tile_lock_guard_vars;
    util::tile_lock *lp = bsg_tile_group_remote_pointer<util::tile_lock>(0, 0, &l);
    static constexpr int n = N;
    for (int i = 0; i < n; i++) {
        util::lock_guard<util::tile_lock> g(*lp);
        x += 1;
    }
    bsg_barrier_tile_group_sync();

    if (__bsg_id == 0)
    {
        TEST_EQ(INT, x, n * bsg_tiles_X * bsg_tiles_Y);  // assert lock works
    }
    bsg_barrier_tile_group_sync();
    return 0;
}

struct Int {
public:
    Int() : v(0) {}
    void add(int x) { v += x; }
    int v;
};

template <typename Lock>
class util::lockable<Int, Lock> {
    UTIL_LOCKABLE_INTERNAL(Int, Lock);
    UTIL_LOCKABLE_METHOD(Int, Lock, add);
    UTIL_LOCKABLE_METHOD_UNSAFE(Int, Lock, add);
};

typedef util::lockable<Int, util::lock> LockableInt;
static DRAM(LockableInt) I, J;

int lockable()
{
    bsg_barrier_tile_group_sync();
    static constexpr int n = N;
    for (int i = 0; i < n; i++) {
        I.add(1);
    }
    bsg_barrier_tile_group_sync();
    for (int i = 0; i < n; i++) {
        J.add_unsafe(1);
    }

    bsg_barrier_tile_group_sync();
    if (__bsg_id == 0)
    {
        TEST_EQ(INT, I.data().v, n * bsg_tiles_X * bsg_tiles_Y);  // assert lock works
        TEST_NEQ(INT, J.data().v, n * bsg_tiles_X * bsg_tiles_Y); // assert lock doesn't work just by accident
    }
    bsg_barrier_tile_group_sync();
    return 0;
}

extern "C" int kernel()
{
    bsg_barrier_tile_group_init();
    bsg_barrier_tile_group_sync();
    lock();
    tile_lock();
    lock_guard();
    tile_lock_guard();
    lockable();
    bsg_barrier_tile_group_sync();
    return 0;
}
