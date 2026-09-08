// Host-only contract regression. Build without -ffast-math, as for the host checker.
#include "../HBNode.hpp"
#include "../force_constants.hpp"
#include "../validation.hpp"
#include <cassert>
#include <limits>
#include <cstdint>
#include <initializer_list>

int main() {
  static_assert(itolsq == 4.0f, "opening criterion must match theta=0.5");
  assert(barnes_hut_error_is_valid(0.0f));
  assert(barnes_hut_error_is_valid(0.01f));
  assert(!barnes_hut_error_is_valid(std::nextafter(0.01f, 1.0f)));
  const float invalid[] = {std::numeric_limits<float>::quiet_NaN(),
                          std::numeric_limits<float>::infinity(),
                          -std::numeric_limits<float>::infinity()};
  for (float value : invalid) {
    assert(!barnes_hut_error_is_valid(value));
    float serror = 0.0f;
    float diff = value - value;
    serror += diff * diff;
    assert(!barnes_hut_error_is_valid(serror));
  }
  // Decoding must preserve every address bit except the tag, including DRAM bit 31.
  for (uint32_t body : {0x80000100u, 0x80001030u, 0x8ffffffcu}) {
    const uint32_t child = body | 1u;
    assert(hb_child_eva(child) == body); // self comparison uses this decoded EVA.
    assert(hb_child_eva(child) % alignof(float) == 0);
    assert(hb_child_eva(body) == body);
  }
}
