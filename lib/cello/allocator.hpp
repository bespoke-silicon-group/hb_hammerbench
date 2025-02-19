#ifndef CELLO_ALLOCATOR_HPP
#define CELLO_ALLOCATOR_HPP
#include <cello/config.hpp>
#include <cstddef>
namespace cello {
/**
 * @brief Initialize allocator
 */
void allocator_initialize(config *cfg);
/**
 * @brief Allocate memory
 * @param size Size of memory to allocate
 * @return Pointer to allocated memory
 */
void *allocate(uintptr_t size);
/**
 * @brief Deallocate memory
 * @param ptr Pointer to memory to deallocate
 * @param size Size of memory to deallocate
 */
void  deallocate(void *ptr, uintptr_t size);
}
#endif
