#ifndef HB_HOST_NUMERIC_H
#define HB_HOST_NUMERIC_H

/* Host-only verification. Integer classification survives -ffast-math;
 * isfinite(x) may be folded to true under finite-math-only assumptions. */
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

static int hb_finite_f32(float x) {
  uint32_t bits;
  memcpy(&bits, &x, sizeof bits);
  /* The volatile integer boundary also prevents the optimizer recognizing
   * the bit test as an isfinite operation and folding it under fast-math. */
  volatile uint32_t observed = bits;
  return (observed & UINT32_C(0x7f800000)) != UINT32_C(0x7f800000);
}

static int hb_finite_f64(double x) {
  uint64_t bits;
  memcpy(&bits, &x, sizeof bits);
  volatile uint64_t observed = bits;
  return (observed & UINT64_C(0x7ff0000000000000)) != UINT64_C(0x7ff0000000000000);
}

typedef struct {
  size_t checked, failures, nonfinite, worst_index;
  double max_abs, max_relative, max_scaled;
} hb_error_stats;

static void hb_record_error(hb_error_stats *s, size_t index, double error,
                            double reference_magnitude, double limit) {
  ++s->checked;
  if (!hb_finite_f64(error) || !hb_finite_f64(reference_magnitude) ||
      !hb_finite_f64(limit) || limit <= 0.0) {
    ++s->nonfinite;
    ++s->failures;
    return;
  }
  if (error > s->max_abs) { s->max_abs = error; s->worst_index = index; }
  /* Diagnostic relative error uses a floor; acceptance uses the named limit. */
  double relative = error / fmax(reference_magnitude, 1e-12);
  if (relative > s->max_relative) s->max_relative = relative;
  double scaled = error / limit;
  if (scaled > s->max_scaled) s->max_scaled = scaled;
  if (error > limit) ++s->failures;
}

static void hb_check_scalar(hb_error_stats *s, size_t index, float actual,
                            double expected, double limit) {
  if (!hb_finite_f32(actual) || !hb_finite_f64(expected)) {
    ++s->checked;
    ++s->nonfinite;
    ++s->failures;
    return;
  }
  hb_record_error(s, index, fabs((double)actual - expected), fabs(expected), limit);
}

static void hb_print_errors(const char *name, const hb_error_stats *s) {
  printf("%s: checked=%zu failures=%zu nonfinite=%zu max_abs=%.9g "
         "worst_index=%zu max_relative=%.9g max_error_over_limit=%.9g\n",
         name, s->checked, s->failures, s->nonfinite, s->max_abs,
         s->worst_index, s->max_relative, s->max_scaled);
}

/* Fixed unsigned arithmetic makes fixtures independent of libc rand(). */
static float hb_fixture_float(uint32_t *state) {
  *state = *state * UINT32_C(1664525) + UINT32_C(1013904223);
  return ((int)((*state >> 8) % 2001) - 1000) / 1000.0f;
}
#endif
