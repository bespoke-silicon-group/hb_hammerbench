#ifndef COMMON_HPP
#define COMMON_HPP
#include <datastructure/csx.hpp>
using csx_type = datastructure::compressed_sparse_matrix<float, int32_t>;
#ifdef HOST
using csx_mirror = typename csx_type::mirror_type;
using eigen_matrix = typename csx_mirror::eigen_sparse_matrix_type;
#endif
#endif
