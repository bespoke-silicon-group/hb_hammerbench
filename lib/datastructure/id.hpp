#ifndef DATASTRUCTURE_ID_HPP
#define DATASTRUCTURE_ID_HPP
#include <global_pointer/global_pointer.hpp>
#ifndef HOST
#include <cello/cello.hpp>
#endif
namespace datastructure
{
/**
 * @brief get the x coordinate of my pod
 */
static inline int pod_x() {
#ifndef HOST
    return cello::my::pod_x();
#else
    throw std::runtime_error("Not implemented");
#endif
    
}
/**
 * @brief get the y coordinate of my pod
 */
static inline int pod_y() {
#ifndef HOST
    return cello::my::pod_y();
#else
    throw std::runtime_error("Not implemented");
#endif
}
/**
 * @brief get the id of my pod
 */
static inline int pod_id() {
#ifndef HOST
    return cello::my::pod_id();
#else
    throw std::runtime_error("Not implemented");
#endif
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
#ifndef HOST
    return cello::my::tile_x();
#else
    throw std::runtime_error("Not implemented");
#endif
}
/**
 * @brief get the y coordinate of my tile in the pod
 */
static inline int tile_y() {
#ifndef HOST
    return cello::my::tile_y();
#else
    throw std::runtime_error("Not implemented");
#endif
}
/**
 * @brief get the id of my tile in the pod
 */
static inline int tile_id() {
#ifndef HOST
    return cello::my::tile_id();
#else
    throw std::runtime_error("Not implemented");
#endif
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
#ifndef HOST
    return cello::my::id();
#else
    throw std::runtime_error("Not implemented");
#endif
}
/**
 * @brief get the number of tiles in the system
 */
static inline int num_tiles_total() {
    return bsg_tiles_X * bsg_tiles_Y * bsg_pods_X * bsg_pods_Y;
}
}
#endif
