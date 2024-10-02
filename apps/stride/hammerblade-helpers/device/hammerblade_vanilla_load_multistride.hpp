#pragma once
#include <cstring>
#include <cstdint>
namespace {

}

namespace hammerblade {
    namespace vanilla {
        // stride size in words
        static constexpr int STRIDE_WORDS = 32;

        // load from a buffer that's multiple strides in size
        // N is a fixed size buffer
        template <int N = STRIDE_WORDS>
        void load_multistride(int *dst, const int *src) {
            constexpr int STRIDES = N/STRIDE_WORDS;
            static_assert(N % STRIDE_WORDS == 0, "N should be a multiple of STRIDE_WORDS");
            // can we unroll this automatically?
#pragma GCC unroll 32
            for (int i = 0; i < STRIDE_WORDS; ++i) {
                // this loop should unroll to initiate multiple fetchs
#pragma GCC unroll 32
                for (int s = 0; s < STRIDES; ++s) {
                    dst[s * STRIDE_WORDS + i] = src[s * STRIDE_WORDS + i];
                }

            }
        }
    }
}
