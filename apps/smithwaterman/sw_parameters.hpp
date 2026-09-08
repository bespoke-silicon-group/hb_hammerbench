#ifndef HB_SW_PARAMETERS_HPP
#define HB_SW_PARAMETERS_HPP

#define NUM_TILES (bsg_tiles_X*bsg_tiles_Y)
#define SEQ_LEN 32
#define DP_LEN (SEQ_LEN+1)
#define NUM_SEQ_PER_TILE (NUM_SEQ/NUM_TILES)
#define NUM_WORD_PER_TILE (SEQ_LEN/4*NUM_SEQ_PER_TILE)

static_assert(NUM_SEQ > 0, "Smith-Waterman requires a positive pair count");
static_assert(NUM_SEQ % NUM_TILES == 0,
              "Smith-Waterman requires a whole number of pairs per tile");
// Local sequence buffers, four DP rows, multipod state, and a 512-byte runtime/stack
// reserve must fit 4 KiB DMEM. Recheck stack usage when changing the kernel.
static_assert(2*SEQ_LEN*NUM_SEQ_PER_TILE + 4*DP_LEN*sizeof(int)
              + (NUM_POD_X+1)*sizeof(int) + 512 <= 4096,
              "Smith-Waterman pair count exceeds per-tile DMEM budget");

#endif
