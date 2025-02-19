#ifndef UTIL_LOCK_HPP
#define UTIL_LOCK_HPP
#include <util/class_field.hpp>
#include <util/backoff.hpp>
#include <atomic>
#include <algorithm>
namespace util
{
/**
 * @brief lock
 */
class lock
{
public:
    /**
     * @brief base constructor
     */
    lock() : locked_(0) {}

    /**
     * @brief acquire the lock
     */
    void acquire()
    {
        exponential_backoff<16>([this]() { return locked_.exchange(1, std::memory_order_acquire) == 1; });
    }

    /**
     * @brief release the lock
     */
    void release()
    {
        locked_.store(0, std::memory_order_release);
    }

    /**
     * @brief locked
     */
    const std::atomic<int>& locked() const { return locked_; }

    /**
     * @brief locked
     */
    std::atomic<int>& locked() { return locked_; }
    
    std::atomic<int> locked_; //!< locked flag
};

/**
 * @brief tile lock, uses the 0/1 lock bit associated with manycore endpoints
 * this lock has no data as the bit is stored in a register
 */
class tile_lock
{
public:
    /**
     * @brief acquire the lock
     */
    void acquire();

    /**
     * @brief release the lock
     */
    void release();
};

/**
 * @brief a lock guard, acquires lock on creation and releases on destruction
 */
template <typename Lock>
class lock_guard
{
public:
    /**
     * @brief constructor
     * @param l lock
     */
    lock_guard(Lock& l) : l_(l) {
        l_.acquire();
    }

    /**
     * @brief destructor
     */
    ~lock_guard() {
        l_.release();
    }

    Lock &l_; //!< lock
};

}
#endif
