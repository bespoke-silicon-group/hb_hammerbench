#ifndef GLOBAL_POINTER_ADDRESS_TYPE_HPP
#define GLOBAL_POINTER_ADDRESS_TYPE_HPP
#include <cstdint>
namespace bsg_global_pointer
{

/**
 * @brief return true if the address is in dram
 */
inline bool is_dram(uintptr_t raw) {
    return (0b1<<DRAM_PREFIX_SHIFT) & raw;
}

/**
 * @brief return true if the address is in dram
 */
template <typename T>
inline bool is_dram(const T *ptr) {
    return is_dram(reinterpret_cast<uintptr_t>(ptr));
}


/**
 * @brief return true if the address is tile group
 */
inline bool is_global_dmem(uintptr_t raw) {
    return ((0b11<<GLOBAL_PREFIX_SHIFT) & raw) == (0b01<<GLOBAL_PREFIX_SHIFT);
}

/**
 * @brief return true if the address is tile group
 */
template <typename T>
inline bool is_global_dmem(const T *ptr) {
    return is_global_dmem(reinterpret_cast<uintptr_t>(ptr));
}


/**
 * @brief return true if the address is tile group
 */
inline bool is_tile_group_dmem(uintptr_t raw) {
    return ((0b111<<REMOTE_PREFIX_SHIFT) & raw) == (0b001<<GLOBAL_PREFIX_SHIFT);
}

/**
 * @brief return true if the address is tile group
 */
template <typename T>
inline bool is_tile_group_dmem(const T *ptr) {
    return is_tile_group_dmem(reinterpret_cast<uintptr_t>(ptr));
}

}
#endif
