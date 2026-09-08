#include <bsg_manycore.h>
#include <bsg_cuda_lite_barrier.h>
#include "bsg_barrier_multipod.h"
#include <cstdint>

#include "sw_parameters.hpp"


// Multipod barrier;
volatile int done[NUM_POD_X]={0};
int alert = 0;


// Local storage;
uint8_t l_query[NUM_SEQ_PER_TILE*SEQ_LEN];
uint8_t l_ref[NUM_SEQ_PER_TILE*SEQ_LEN];
int E_spm[DP_LEN];
int F_spm[DP_LEN];
int H_spm[DP_LEN];
int H_prev_spm[DP_LEN];


inline int max(int a, int b) {
  if (a>b) 
    return a;
  else
    return b;
}

inline int max(int a, int b, int c) {
  return max(a, max(b,c));
}

inline int max(int a, int b, int c, int d) {
  return max(max(a,b), max(c,d));
}

// Hyper params;
#define MATCH_SCORE     1
#define MISMATCH_SCORE -3
#define GAP_OPEN        3
#define GAP_EXTEND      1
inline void align(uint8_t* seqa, uint8_t* seqb, int* output) {
  int score_temp = 0;
  for (int i = 1; i <= SEQ_LEN; i++) {
    for (int j = 1; j < DP_LEN; j++) {
      E_spm[j] = max(E_spm[j-1] - GAP_EXTEND,
                     H_spm[j-1] - GAP_OPEN);
      F_spm[j] = max(F_spm[j] - GAP_EXTEND,
                     H_spm[j] - GAP_OPEN);
      H_prev_spm[j] = H_spm[j];
      H_spm[j] = max(0,
                     E_spm[j],
                     F_spm[j],
                     H_prev_spm[j-1] + ((seqa[i-1] == seqb[j-1]) ? MATCH_SCORE : MISMATCH_SCORE));
      if (H_spm[j] > score_temp) {
        score_temp = H_spm[j];
      }
    }
  }

  // Write result;
  *output = score_temp;
}

// Kernel main;
extern "C" int kernel(uint8_t* query, uint8_t* ref, int* output, int pod_id)
{
  bsg_barrier_tile_group_init();
  bsg_barrier_tile_group_sync();

  // prefetch;
  int* query_word = (int*) query;
  int* ref_word = (int*) ref;
  int* l_query_word = (int*) l_query;
  int* l_ref_word = (int*) l_ref;

  for (int i = 0; i < NUM_WORD_PER_TILE; i++) {
    l_query_word[i] = query_word[(__bsg_id*NUM_WORD_PER_TILE)+i];
    l_ref_word[i] = ref_word[(__bsg_id*NUM_WORD_PER_TILE)+i];
  }
  
  for (int i = 0; i < NUM_SEQ_PER_TILE; i++) {
    asm volatile ("sw x0, %[p]" :: [p] "m" (output[i+(__bsg_id*NUM_SEQ_PER_TILE)]));
  }
  bsg_fence();


  // kernel start;
  bsg_barrier_multipod(pod_id, NUM_POD_X, done, &alert);
  bsg_cuda_print_stat_kernel_start();


  bsg_unroll(1)
  for (int i = 0; i < NUM_SEQ_PER_TILE; i++) {
    // Clear the zero boundary and all 32 character positions.
    for (int j = 0; j < DP_LEN; j++) {
      E_spm[j] = 0;
      F_spm[j] = 0;
      H_spm[j] = 0;
      H_prev_spm[j] = 0;
    }
    // align;
    align(&l_query[i*SEQ_LEN], &l_ref[i*SEQ_LEN], &output[(__bsg_id*NUM_SEQ_PER_TILE)+i]);
  }


  // kernel end;
  bsg_fence();
  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_tile_group_sync();
  return 0;
}
