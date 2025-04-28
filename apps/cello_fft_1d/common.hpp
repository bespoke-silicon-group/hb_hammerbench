#pragma once
#include <complex>
#include <datastructure/vector.hpp>

using FP32Complex = std::complex<float>;

using point_vector    = datastructure::vector<FP32Complex>;
using position_vector = datastructure::vector<int>;
