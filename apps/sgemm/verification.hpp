#ifndef HB_SGEMM_VERIFICATION_HPP
#define HB_SGEMM_VERIFICATION_HPP
#include "../common/host_numeric.h"

/* Independent scalar FP64 dot products of the actual FP32 inputs.
 * The sum of absolute products supplies a cancellation-aware error bound. */
static void sgemm_reference(const float *a, const float *b, double *out,
                            double *sum_abs, int n, int iterations) {
  for (int batch = 0; batch < iterations; ++batch) {
    size_t base = (size_t)batch * n * n;
    for (int row = 0; row < n; ++row) {
      for (int col = 0; col < n; ++col) {
        double sum = 0.0, magnitude = 0.0;
        for (int k = 0; k < n; ++k) {
          double product = (double)a[base + row*n + k] * b[base + k*n + col];
          sum += product;
          magnitude += fabs(product);
        }
        out[base + row*n + col] = sum;
        sum_abs[base + row*n + col] = magnitude;
      }
    }
  }
}

static hb_error_stats sgemm_verify(const float *actual, const double *expected,
                                   const double *sum_abs, size_t count, int n) {
  hb_error_stats stats = {};
  /* Four times the usual gamma_n FP32 accumulation bound, plus an absolute
   * floor for tiny answers. This is a verification policy, not a measured
   * accuracy claim. n*u < 1 is required by the bound. */
  const double nu = n / 16777216.0;
  if (n <= 0 || nu >= 1.0) { stats.failures = 1; return stats; }
  const double gamma = nu / (1.0 - nu);
  for (size_t i = 0; i < count; ++i)
    hb_check_scalar(&stats, i, actual[i], expected[i],
                    1e-6 + 4.0 * gamma * sum_abs[i]);
  return stats;
}
#endif
