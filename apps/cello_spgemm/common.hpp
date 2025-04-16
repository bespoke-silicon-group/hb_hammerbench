#ifndef COMMON_HPP
#define COMMON_HPP
#include <datastructure/csx.hpp>
#include <util/class_field.hpp>
#include <util/list.hpp>
#ifndef HOST
#include <utility>
#include "rb_tree.hpp"
#include <cello/cello.hpp>
#endif

/* data value type */
using value_type = float;

/* indexing type */
using index_type = int32_t;

/* nonzero type */
struct nonzero_type {
    FIELD(index_type, idx);
    FIELD(value_type, val);
};

#ifndef HOST
using partial_table = rb_tree<index_type, value_type>;
#else
struct partial_table {
    hb_mc_eva_t root = 0;
    uint32_t    size = 0;
};
#endif

/* compressed sparse matrix type */
using csx_type = datastructure::compressed_sparse_matrix<value_type, index_type, datastructure::CSX_FLAG_ROW_MAJOR>;

/* partial table vector type */
using partial_table_vector = datastructure::vector<partial_table, 1>;

#ifdef HOST
using csx_mirror = csx_type::mirror_type;
using partial_table_vector_mirror = partial_table_vector::mirror_type;
#endif




#endif
