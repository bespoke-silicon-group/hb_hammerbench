#ifndef CELLO_POINTER_HPP
#define CELLO_POINTER_HPP
#include <global_pointer/global_pointer.hpp>
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

}
#endif
