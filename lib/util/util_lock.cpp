#include <util/lock.hpp>
#include <util/backoff.hpp>
#include <bsg_manycore.hpp>
#include <bsg_manycore.h>
#include "bsg_manycore_atomic.h"
#include "bsg_tile_config_vars.h"

namespace util
{

tile_lock::tile_lock() {
    //global_this() = bsg_tile_group_remote_pointer<tile_lock>(__bsg_x, __bsg_y, this);
}

/**
 * @brief acquire the lock
 */
void tile_lock::acquire()
{
    int *lp = reinterpret_cast<int*>(this);

    exponential_backoff<16>([=]() { return bsg_amoswap(lp, 1) == 1; });
}

/**
 * @brief release the lock
 */
void tile_lock::release()
{
    int *lp = reinterpret_cast<int*>(this);
    bsg_amoswap(lp, 0);
}

}

