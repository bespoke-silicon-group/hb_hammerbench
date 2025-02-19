#include <cello/allocator.hpp>
#include <atomic>
#include <bsg_manycore.h>
#include <bsg_cuda_lite_barrier.h>
namespace cello
{

__attribute__((section(".dram")))
std::atomic<uintptr_t> allocator_base;
uintptr_t allocator_end;

/**
 * @brief Initialize allocator
 */
void allocator_initialize(config *cfg) {
    if (__bsg_id == 0) {
        allocator_base = cfg->dram_buffer();
        allocator_end = allocator_base + cfg->dram_buffer_size();
    }
}

/**
 * @brief Allocate memory
 * @param size Size of memory to allocate
 * @return Pointer to allocated memory
 */
void *allocate(size_t size) {
    uintptr_t mem = allocator_base.fetch_add(size, std::memory_order_relaxed);
    if (mem + size > allocator_end) {
        bsg_print_hexadecimal(mem);
        bsg_print_hexadecimal(0xdeadbeef);
        while (1);
    }
    return reinterpret_cast<void *>(mem);
}

/**
 * @brief Deallocate memory
 * @param ptr Pointer to memory to deallocate
 * @param size Size of memory to deallocate
 */
void  deallocate(void *ptr, size_t size)
{
    // do nothing for now
}

}
