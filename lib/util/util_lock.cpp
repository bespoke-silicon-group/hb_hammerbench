#ifndef CELLO_ICACHE_OPT
#include <util/lock.hpp>
namespace util
{
/**
 * @brief acquire the lock
 */
void lock::acquire()
{
#ifndef UTIL_LOCK_NO_EXPONENTIAL_BACKOFF
    exponential_backoff<16>([this]() { return locked_.exchange(1, std::memory_order_acquire) == 1; });
#else
    while (locked_.exchange(1, std::memory_order_acquire) == 1);
#endif
}

/**
 * @brief try to acquire the lock
 * return true if the lock is acquired.
 */
bool lock::try_acquire()
{
    int l = locked_.exchange(1, std::memory_order_acquire);
    return l == 0;
}

/**
 * @brief release the lock
 */
void lock::release()
{
    locked_.store(0, std::memory_order_release);
}


/**
 * @brief acquire the lock
 */
void tile_lock::acquire() {
    int *lp = reinterpret_cast<int*>(this);
#ifndef UTIL_LOCK_NO_EXPONENTIAL_BACKOFF
    exponential_backoff<16>([=]() { return bsg_amoswap(lp, 1) == 1; });
#else
    while (bsg_amoswap(lp, 1) == 1);
#endif
}

/**
 * @brief try to acquire the lock
 * return true if the lock is acquired
 */
bool tile_lock::try_acquire() {
    int *lp = reinterpret_cast<int*>(this);
    int l = bsg_amoswap(lp, 1);
    return l == 0;
}

/**
 * @brief release the lock
 */
void tile_lock::release() {
    int *lp = reinterpret_cast<int*>(this);
    bsg_amoswap_rl(lp, 0);
}
}
#endif
