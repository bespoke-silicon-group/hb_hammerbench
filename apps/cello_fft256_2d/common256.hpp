#pragma once
#ifndef HOST
#include "fft256.hpp"
#endif
#include <array>
#include <complex>
#include <datastructure/vector.hpp>

#ifndef MAX_NUM_POINTS
#define MAX_NUM_POINTS 256
#endif

#ifndef HOST
#else
using FP32Complex = std::complex<float>;
#endif

using vector = std::array<FP32Complex, MAX_NUM_POINTS>;
using matrix = std::array<vector, MAX_NUM_POINTS>;
using batch_vector = datastructure::vector<matrix, 1>;
