#ifndef CELLO_THREADS_HPP
#define CELLO_THREADS_HPP
namespace cello {

/**
 * @brief get the number of threads in the system
 */
static inline int threads() {
#ifdef CELLO_GLOBAL_STEALING
    return bsg_tiles_X*bsg_tiles_Y*bsg_pods_X*bsg_pods_Y;
#else
    return bsg_tiles_X*bsg_tiles_Y;
#endif
}

}
#endif
