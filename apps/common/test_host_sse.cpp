#include "host_sse.hpp"
#include <cstdlib>

// Fault operands come from integer object representations, not FP NaN predicates.
static float from_bits(uint32_t bits) {
  float value;
  std::memcpy(&value, &bits, sizeof value);
  return value;
}

int main() {
  const float limits[] = {0.001f, 0.01f};
  volatile uint32_t fault_bits[] = {0x7fc00000u, 0x7f800000u, 0xff800000u};
  unsigned checks = 0;
  for (float limit : limits) {
    hb_host_sse exact;
    exact.add(0.25f, 0.25f, "exact", 0, 0);
    if (!exact.accepts(limit) || exact.sum != 0.0f) return 1;
    hb_host_sse boundary;
    boundary.sum = limit;
    if (!boundary.accepts(limit)) return 2;
    boundary.sum = limit * 2.0f;
    if (boundary.accepts(limit)) return 3;
    hb_host_sse small;
    small.add(0.03125f, 0.0f, "small", 0, 0);
    if (!small.accepts(limit) || small.sum != 0.0009765625f) return 4;
    hb_host_sse wrong;
    wrong.add(1.0f, 0.0f, "wrong-finite", 0, 0);
    if (wrong.accepts(limit)) return 5;
    checks += 5;
    for (unsigned i = 0; i < 3; ++i) {
      float bad = from_bits(fault_bits[i]);
      for (unsigned side = 0; side < 3; ++side) {
        hb_host_sse error;
        error.add(side != 1 ? bad : 0.0f, side != 0 ? bad : 0.0f,
                  "injected", 2, int(i * 3 + side));
        if (error.accepts(limit) || error.invalid != 1) return 6;
        ++checks;
      }
    }
    hb_host_sse overflow;
    overflow.add(from_bits(0x7f7fffffu), -from_bits(0x7f7fffffu), "overflow", 0, 0);
    if (overflow.accepts(limit) || !overflow.invalid) return 7;
    hb_host_sse accumulation;
    for (int i = 0; i < 4; ++i)
      accumulation.add(1.0e19f, 0.0f, "sum-overflow", 0, i);
    if (accumulation.accepts(limit) || !accumulation.invalid) return 8;
    checks += 2;
  }
  std::printf("PASS: %u finite/SSE checks at original PageRank and Black-Scholes thresholds\n", checks);
}
