#ifndef HB_FFT_VERIFICATION_H
#define HB_FFT_VERIFICATION_H
#include "../../common/host_numeric.h"
#include <complex.h>
#include <stdlib.h>

typedef struct { double real, imag; } fft_reference_value;

/* General scalar FP64 radix-2 transform of the exact FP32 input. This does
 * not use the device's tables, bit-reversal lists, factored P-by-P layout,
 * twiddle matrix, or generated butterfly code. Tests also compare it with
 * a direct DFT and analytic impulse/constant transforms. */
static int fft_reference(const float complex *in, fft_reference_value *out, int n) {
  if (n <= 0 || (n & (n-1))) return 1;
  for (int i = 0, j = 0; i < n; ++i) {
    out[j].real = crealf(in[i]);
    out[j].imag = cimagf(in[i]);
    int bit = n >> 1;
    while (bit && (j & bit)) { j ^= bit; bit >>= 1; }
    j ^= bit;
  }
  const double pi = acos(-1.0);
  for (int len = 2; len <= n; len *= 2) {
    for (int k = 0; k < len/2; ++k) {
      double angle = -2.0*pi*k/len;
      double wr = cos(angle), wi = sin(angle);
      for (int base = 0; base < n; base += len) {
        int a = base+k, b = a+len/2;
        double tr = wr*out[b].real - wi*out[b].imag;
        double ti = wr*out[b].imag + wi*out[b].real;
        double ar = out[a].real, ai = out[a].imag;
        out[a].real = ar+tr; out[a].imag = ai+ti;
        out[b].real = ar-tr; out[b].imag = ai-ti;
      }
    }
    if (len == n) break;
  }
  return 0;
}

static void fft_fixture(float complex *in, int n, const char *name) {
  uint32_t state = 1;
  for (int i = 0; i < n; ++i) {
    if (!strcmp(name, "impulse")) in[i] = i == 37 % n ? 1.0f-0.5f*I : 0.0f;
    else if (!strcmp(name, "constant")) in[i] = 0.25f-0.125f*I;
    else if (!strcmp(name, "random")) {
      float real = hb_fixture_float(&state), imag = hb_fixture_float(&state);
      in[i] = real + I*imag;
    }
    else in[i] = cosf(i*acos(-1.0)/8.0); /* Original expert input. */
  }
}

static hb_error_stats fft_verify_strong(const float complex *actual,
                                       const fft_reference_value *ref, int n) {
  hb_error_stats stats = {0};
  for (int i = 0; i < n; ++i) {
    float ar = crealf(actual[i]), ai = cimagf(actual[i]);
    if (!hb_finite_f32(ar) || !hb_finite_f32(ai) ||
        !hb_finite_f64(ref[i].real) || !hb_finite_f64(ref[i].imag)) {
      ++stats.checked; ++stats.nonfinite; ++stats.failures;
      continue;
    }
    double magnitude = hypot(ref[i].real, ref[i].imag);
    double error = hypot((double)ar-ref[i].real, (double)ai-ref[i].imag);
    hb_record_error(&stats, i, error, magnitude, 5e-4 + 2e-5*magnitude);
  }
  return stats;
}
#endif
