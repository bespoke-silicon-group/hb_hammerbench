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
}
#endif
