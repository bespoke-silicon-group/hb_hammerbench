#ifndef CELLO_THREAD_ID_HPP
#define CELLO_THREAD_ID_HPP
#include <bsg_manycore.h>
#include <bsg_tile_config_vars.h>
#include <cello/config.hpp>
namespace cello {
namespace my {
extern int __bsg_pod_x, __bsg_pod_y, __bsg_pod_id, __bsg_absolute_id;

/**
 * @brief get the x coordinate of my pod
 */
static inline int pod_x() {
    return __bsg_pod_x;
}
/**
 * @brief get the y coordinate of my pod
 */
static inline int pod_y() {
    return __bsg_pod_y;
}
/**
 * @brief get the id of my pod
 */
static inline int pod_id() {
    return __bsg_pod_id;
}
/**
 * @brief get the number of pods x
 */
static inline int num_pods_x() {
    return bsg_pods_X;
}
/**
 * @brief get the number of pods y
 */
static inline int num_pods_y() {
    return bsg_pods_Y;
}
/**
 * @brief get the number of pods
 */
static inline int num_pods() {
    return bsg_pods_X * bsg_pods_Y;
}
/**
 * @brief get the x coordinate of my tile in the pod
 */
static inline int tile_x() {
    return __bsg_x;
}
/**
 * @brief get the y coordinate of my tile in the pod
 */
static inline int tile_y() {
    return __bsg_y;
}
/**
 * @brief get the id of my tile in the pod
 */
static inline int tile_id() {
    return __bsg_id;
}
/**
 * @brief get the number of tiles x in the pod
 */
static inline int num_tiles_x() {
    return bsg_tiles_X;
}
/**
 * @brief get the number of tiles y in the pod
 */
static inline int num_tiles_y() {
    return bsg_tiles_Y;
}
/**
 * @brief get the number of tiles in the pod
 */
static inline int num_tiles() {
    return bsg_tiles_X * bsg_tiles_Y;
}
/**
 * @brief get the absolute id
 */
static inline int id() {
    return __bsg_absolute_id;
}
}

/**
 * @brief setup the id variables
 */
void set_id_vars(config *cfg);
}
#endif
