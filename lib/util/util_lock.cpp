#include <util/lock.hpp>
#include <util/backoff.hpp>
#include "bsg_manycore_atomic.h"

namespace util
{

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

