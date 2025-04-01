#include <cello/allocator.hpp>
#include <cello/thread_id.hpp>
#include <atomic>
#include <bsg_manycore.h>
#include <cstring>
#include <algorithm>

namespace cello
{

__attribute__((section(".dram")))
std::atomic<uintptr_t> allocator_base;

__attribute__((section(".dram")))
uintptr_t allocator_end;

/**
 * @brief Initialize allocator
 */
void allocator_initialize(config *cfg) {
    if (my::tile_id() == 0) {
        bsg_print_hexadecimal(cfg->dram_buffer());
        bsg_print_hexadecimal(0xcafebabe);
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
    // align size to size of cache line
    size = (size + BSG_CACHE_LINE_SIZE - 1) & ~(BSG_CACHE_LINE_SIZE - 1);
    // allocate memory
    uintptr_t mem = allocator_base.fetch_add(size, std::memory_order_relaxed);
    if (mem + size > allocator_end) {
        bsg_print_hexadecimal(mem);
        bsg_print_hexadecimal(0xdeadbeef);
        while (1);
    }
    memset(reinterpret_cast<void *>(mem), 0, size);
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
