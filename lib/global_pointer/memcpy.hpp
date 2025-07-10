#ifndef GLOBAL_POINTER_MEMCPY_HPP
#define GLOBAL_POINTER_MEMCPY_HPP
#include <global_pointer/reference.hpp>
#include <global_pointer/pointer.hpp>
#include <cstddef>
namespace bsg_global_pointer {
#ifndef BSG_GLOBAL_POINTER_OPT_MEMCPY
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
#else
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
    size_t i = 0;

    for (; i + 16 <= n; i += 16) {
        int *dstw = reinterpret_cast<int*>(dst);
        int* srcw = reinterpret_cast<int*>(src.to_local());
        register int r[4];
        {
            pod_address_guard _ (src.pod_addr());
            r[0] = srcw[0];
            r[1] = srcw[1];
            r[2] = srcw[2];
            r[3] = srcw[3];
            asm volatile ("" ::: "memory");
        }
        dstw[0] = r[0];
        dstw[1] = r[1];
        dstw[2] = r[2];
        dstw[3] = r[3];
        asm volatile ("" ::: "memory");
        dst += 16;
        src += 16;
    }

    for (; i + 4 <= n; i += 4) {
        int *dstw = reinterpret_cast<int*>(dst);
        int* srcw = reinterpret_cast<int*>(src.to_local());
        register int r[1];
        {
            pod_address_guard _ (src.pod_addr());
            r[0] = srcw[0];
            asm volatile ("" ::: "memory");
        }
        dstw[0] = r[0];
        asm volatile ("" ::: "memory");
        dst += 4;
        src += 4;
    }

    for (; i < n; i++) {
        dst[i] = src[i];
    }
}
#endif
}
#endif
