#ifndef COMMON_HPP
#define COMMON_HPP
#include <datastructure/vector.hpp>
#include <array>
#include <cstdint>
#define NUM_TILES (bsg_tiles_X*bsg_tiles_Y*bsg_pods_X*bsg_pods_Y)
#define SEQ_LEN 32
#define NUM_SEQ_PER_TILE (NUM_SEQ/NUM_TILES)
#define NUM_WORD_PER_TILE (SEQ_LEN/4*NUM_SEQ/NUM_TILES)

typedef std::array<uint8_t, SEQ_LEN> sequence;
typedef int32_t score;

typedef datastructure::vector<sequence, 1> input_vector;
typedef datastructure::vector<score, 1> output_vector;
#endif
