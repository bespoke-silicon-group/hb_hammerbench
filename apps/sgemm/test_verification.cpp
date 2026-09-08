#include "verification.hpp"
#include <vector>

int main() {
  const int n = 16, batches = 2;
  const size_t count = n*n*batches;
  std::vector<float> a(count), b(count), actual(count);
  std::vector<double> ref(count), magnitude(count);
  uint32_t seed = 1;
  for (size_t i = 0; i < count; ++i) {
    a[i] = hb_fixture_float(&seed);
    b[i] = hb_fixture_float(&seed);
  }
  sgemm_reference(a.data(), b.data(), ref.data(), magnitude.data(), n, batches);
  /* Independent column-wise long-double reference checks every output. */
  for (int batch = 0; batch < batches; ++batch)
    for (int col = 0; col < n; ++col)
      for (int row = 0; row < n; ++row) {
        long double expected = 0;
        size_t base = batch*n*n, i = base + row*n + col;
        for (int k = 0; k < n; ++k)
          expected += (long double)a[base+row*n+k] * b[base+k*n+col];
        if (fabs(ref[i] - (double)expected) > 1e-12) return 1;
        actual[i] = (float)expected;
      }
  if (sgemm_verify(actual.data(), ref.data(), magnitude.data(), count, n).failures) return 2;
  const uint32_t faults[] = {0x7fc00000, 0x7f800000, 0xff800000, 0x3f800000};
  const char *names[] = {"NaN", "+Inf", "-Inf", "wrong-finite"};
  for (int k = 0; k < 4; ++k) {
    float saved = actual[17];
    memcpy(&actual[17], &faults[k], sizeof(float));
    hb_error_stats s = sgemm_verify(actual.data(), ref.data(), magnitude.data(), count, n);
    printf("SGEMM fault=%s rejected=%d nonfinite=%zu\n", names[k], !!s.failures, s.nonfinite);
    if (!s.failures || (k < 3 && s.nonfinite != 1)) return 3;
    actual[17] = saved;
  }
  puts("SGEMM verification tests PASSED");
  return 0;
}
