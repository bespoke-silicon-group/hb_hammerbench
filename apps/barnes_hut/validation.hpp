#ifndef BARNES_HUT_VALIDATION_HPP
#define BARNES_HUT_VALIDATION_HPP

#include <cmath>

// Preserve the expert SSE threshold; non-finite sums cannot establish agreement.
inline bool barnes_hut_error_is_valid(float serror) {
  return std::isfinite(serror) && serror <= 0.01f;
}

#endif
