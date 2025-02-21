#ifndef CELLO_THREADS_HPP
#define CELLO_THREADS_HPP
namespace cello {

/**
 * @brief get the number of threads in the system
 */
static inline int threads() {
    return bsg_tiles_X*bsg_tiles_Y;
}

}
#endif
