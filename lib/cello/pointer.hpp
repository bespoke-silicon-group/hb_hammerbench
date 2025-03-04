#ifndef CELLO_POINTER_HPP
#define CELLO_POINTER_HPP
#include <global_pointer/global_pointer.hpp>
#include <cello/thread_id.hpp>
#include <bsg_manycore.hpp>
namespace cello
{
/**
 * @brief pointer
 */
template <typename T>
using global_pointer = bsg_global_pointer::pointer<T>;

/**
 * @brief return true if pointer is null
 */
template <typename T>
static inline bool is_null(T * p) {
    return p == nullptr;
}

/**
 * @brief return true if pointer is null
 */
template <typename T>
static inline bool is_null(global_pointer<T> p) {
    return p.ref().addr().raw() == 0;
}

/**
 * @brief return the global pointer of a local variable (dmem)
 */
template <typename T>
static inline global_pointer<T> addressof_localvar(T& t) {
    T*p = bsg_tile_group_remote_pointer<T>(my::tile_x(), my::tile_y(), &t);
    return global_pointer<T>::onPodXY(my::pod_x(), my::pod_y(), p);
}

/**
 * @brief return the global pointer of a global variable (dram)
 */
template <typename T>
static inline global_pointer<T> addressof_globalvar(T& t) {
    T*p = &t;
    return global_pointer<T>::onPodXY(my::pod_x(), my::pod_y(), p);
}

}
#endif
