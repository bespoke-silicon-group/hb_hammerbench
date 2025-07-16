#ifndef COMMON_HPP
#define COMMON_HPP
#include <datastructure/vector.hpp>
#include "option_data.hpp"

#define CHUNK_SIZE 4

using vector_type = datastructure::vector<OptionData, CHUNK_SIZE>;
#endif
