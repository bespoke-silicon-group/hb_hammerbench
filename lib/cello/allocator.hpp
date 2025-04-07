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

/**
 * @brief An allocator object that uses the global cello allocator
 */
template <typename T>
class allocator
{
public:
    using value_type = T;

    /**
     * @brief Default constructor
     */
    allocator () noexcept = default;

    /**
     * @brief Copy constructor
     * @param other the allocator to copy
     */
    allocator (const allocator&) noexcept = default;

    /**
     * @brief Move constructor
     * @param other the allocator to move
     */
    allocator (allocator&&) noexcept = default;

    /**
     * @brief Destructor
     */
    ~allocator() noexcept = default;

    template <typename U>
    allocator (const allocator<U>&) noexcept
    {
        // This constructor is not used in the current implementation
        // but is required to be defined for the allocator to be usable
        // with STL containers.
    }

    /**
     * @brief Allocate memory for an array of T
     * @param size the number of elements to allocate
     */
    T* allocate(size_t size) noexcept
    {
        if (size == 0) {
            return nullptr;
        }
        return static_cast<T*>(cello::allocate(size * sizeof(T)));
    }

    /**
     * @brief Deallocate memory for an array of T
     * @param ptr the pointer to the memory to deallocate
     * @param size the number of elements to deallocate
     */
    void deallocate(T* ptr, size_t size) noexcept
    {
        if (ptr == nullptr) {
            return;
        }
        cello::deallocate(ptr, size * sizeof(T));
    }
};
}
#endif
