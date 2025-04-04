#ifndef COMMON_HPP
#define COMMON_HPP
#include <datastructure/csx.hpp>
#include <util/class_field.hpp>
#include "hb_list.hpp"

/* data value type */
using value_type = float;

/* indexing type */
using index_type = int32_t;

/* nonzero type */
struct nonzero_type {
    FIELD(index_type, idx);
    FIELD(value_type, val);
};

/* compressed sparse matrix type */
using csx_type = datastructure::compressed_sparse_matrix<value_type, index_type, datastructure::CSX_FLAG_ROW_MAJOR>;

/* stores intermediate results for a row */
using list_head_vector = datastructure::vector<HBListNodePtr, 1>;

/* stores the number of nonzeros in a row */
using count_vector = datastructure::vector<index_type, 1>;

#ifdef HOST
using csx_mirror = csx_type::mirror_type;
using list_head_vector_mirror = list_head_vector::mirror_type;
using count_vector_mirror = count_vector::mirror_type;
#endif




#endif
