#ifndef COMMON_HPP
#define COMMON_HPP
#include "HBBody.hpp"
#include "HBNode.hpp"
#include <datastructure/vector.hpp>
#include "bsg_manycore.h"

using body_vector = datastructure::vector<HBBody>;
using node_vector = datastructure::vector<HBNode>;
#ifdef HOST
struct global_pointer {
    uint16_t    pod;
    hb_mc_eva_t raw;
};
using global_node_pointer = global_pointer;
#else
using global_node_pointer = bsg_global_pointer::pointer<HBNode>;
#endif

#endif
