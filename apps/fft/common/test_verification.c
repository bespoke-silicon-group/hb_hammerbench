#include "verification.h"

int main(void) {
  const int n = 64;
  float complex in[n], actual[n];
  fft_reference_value ref[n];
  const char *fixtures[] = {"impulse", "constant", "random", "expert"};
  for (int f = 0; f < 4; ++f) {
    fft_fixture(in, n, fixtures[f]);
    if (fft_reference(in, ref, n)) return 1;
    for (int k = 0; k < n; ++k) {
      /* Direct O(n^2) DFT: different evaluation structure from both FFTs. */
      double real = 0, imag = 0;
      for (int j = 0; j < n; ++j) {
        double angle = -2*acos(-1.0)*k*j/n;
        real += crealf(in[j])*cos(angle) - cimagf(in[j])*sin(angle);
        imag += crealf(in[j])*sin(angle) + cimagf(in[j])*cos(angle);
      }
      if (hypot(real-ref[k].real, imag-ref[k].imag) > 1e-10) return 2;
      if (f == 0) {
        double angle = -2*acos(-1.0)*k*37/n;
        if (hypot(ref[k].real-(cos(angle)+0.5*sin(angle)),
                  ref[k].imag-(sin(angle)-0.5*cos(angle))) > 1e-12) return 3;
      }
      if (f == 1 && hypot(ref[k].real-(k == 0 ? n*0.25 : 0),
                          ref[k].imag-(k == 0 ? -n*0.125 : 0)) > 1e-12) return 4;
      actual[k] = (float)real + I*(float)imag;
    }
    if (fft_verify_strong(actual, ref, n).failures) return 5;
    printf("FFT independent-reference fixture=%s PASSED\n", fixtures[f]);
  }
  const uint32_t faults[] = {0x7fc00000, 0x7f800000, 0xff800000, 0x3f800000};
  const char *names[] = {"NaN", "+Inf", "-Inf", "wrong-finite"};
  for (int k = 0; k < 4; ++k) {
    float complex saved = actual[17];
    /* C complex has the representation of two component floats. Inject
     * directly so complex arithmetic cannot alter the NaN/Inf payload. */
    memcpy((char *)&actual[17] + sizeof(float), &faults[k], sizeof(float));
    hb_error_stats s = fft_verify_strong(actual, ref, n);
    printf("FFT fault=%s rejected=%d nonfinite=%zu\n", names[k], !!s.failures, s.nonfinite);
    if (!s.failures || (k < 3 && s.nonfinite != 1)) return 6;
    actual[17] = saved;
  }
  puts("FFT verification tests PASSED");
  return 0;
}
