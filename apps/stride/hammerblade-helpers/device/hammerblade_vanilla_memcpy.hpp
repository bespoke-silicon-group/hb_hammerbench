#pragma once
#include <cstring>
#include <cstdint>
namespace {
    template <unsigned int FACTOR>
        void *__bsg_memcpy(void *dest,
                           const void *src,
                           const size_t n){

                const float *psrc  asm ("x10") = reinterpret_cast<const float *>(src);
                uint32_t src_nelements = n / sizeof(psrc[0]);
                float *pdest asm ("x12") = reinterpret_cast<float *>(dest);

                for(int j = 0; j < src_nelements; j+= FACTOR){
                        float rtemp[FACTOR];
#pragma GCC unroll 32
                        for(int f = 0; f < FACTOR; f++){
                                asm volatile ("flw %0,%1" : "=f" (rtemp[f]) : "m" (psrc[f]));
                        }

                        // Write
#pragma GCC unroll 32
                        for(int f = 0; f < FACTOR; f++){
                                asm volatile ("fsw %1,%0" : "=m" (pdest[f]) : "f" (rtemp[f]));
                        }
                        psrc += FACTOR;
                        pdest += FACTOR;
                }
                return dest;
        }
}

namespace hammerblade {
    namespace vanilla {
        template <unsigned int FACTOR=32>
        void *memcpy(void * __restrict dst, const void * __restrict src, const size_t n) {
            uintptr_t
                udst = reinterpret_cast<uintptr_t>(dst),
                usrc = reinterpret_cast<uintptr_t>(src);

            static const unsigned int C_WORD_MASK = 0x3;
            static constexpr unsigned int _FACTOR = FACTOR > 32 ? 32 : FACTOR;
            if ((n & C_WORD_MASK) | (udst & C_WORD_MASK) | (usrc & C_WORD_MASK)){
                return std::memcpy(dst, src, n);
            } else {
                return __bsg_memcpy<_FACTOR>(dst, src, n);
            }
        }
    }
}
