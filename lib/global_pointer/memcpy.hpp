#ifndef GLOBAL_POINTER_MEMCPY_HPP
#define GLOBAL_POINTER_MEMCPY_HPP
#include <global_pointer/reference.hpp>
#include <global_pointer/pointer.hpp>
#include <cstddef>
namespace bsg_global_pointer {

inline void memcpy(pointer<char> dst, pointer<char> src, size_t n) {
    for (size_t i = 0; i < n; i++) {
        dst[i] = src[i];
    }
}

inline void memcpy(pointer<char> dst, const char *src, size_t n) {
    for (size_t i = 0; i < n; i++) {
        dst[i] = src[i];
    }
}

inline void memcpy(char *dst, pointer<char> src, size_t n) {
    for (size_t i = 0; i < n; i++) {
        dst[i] = src[i];
    }
}
}
#endif
