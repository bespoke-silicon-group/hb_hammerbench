#ifndef HB_HOST_SSE_HPP
#define HB_HOST_SSE_HPP

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>

// Host-only binary32 classification. A volatile integer boundary prevents
// -ffast-math from recognizing/folding this into an assumed-true isfinite call.
inline bool hb_sse_finite(float value) {
  static_assert(sizeof(float) == sizeof(uint32_t) &&
                std::numeric_limits<float>::is_iec559 &&
                std::numeric_limits<float>::digits == 24,
                "host SSE verification requires IEEE binary32 float");
  uint32_t bits;
  std::memcpy(&bits, &value, sizeof bits);
  volatile uint32_t observed = bits;
  return (observed & UINT32_C(0x7f800000)) != UINT32_C(0x7f800000);
}

struct hb_host_sse {
  float sum;
  unsigned invalid;

  hb_host_sse() : sum(0.0f), invalid(0) {}

  void add(float actual, float expected, const char* component, int pod, int index) {
    if (!hb_sse_finite(actual) || !hb_sse_finite(expected)) {
      if (invalid == 0)
        std::printf("non-finite %s: pod=%d index=%d actual=%g expected=%g\n",
                    component, pod, index, actual, expected);
      ++invalid;
      return;
    }
    const float diff = actual - expected;
    sum += diff * diff;
    if (!hb_sse_finite(sum)) {
      if (invalid == 0)
        std::printf("non-finite %s SSE: pod=%d index=%d actual=%g expected=%g sum=%g\n",
                    component, pod, index, actual, expected, sum);
      ++invalid;
    }
  }

  bool accepts(float threshold) const {
    return invalid == 0 && hb_sse_finite(sum) && sum <= threshold;
  }
};

#endif
