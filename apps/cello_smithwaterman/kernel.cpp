#include <cello/cello.hpp>
#include <util/statics.hpp>
#include "common.hpp"

DRAM(input_vector) query;
DRAM(input_vector) ref;
DRAM(output_vector) output;

DMEM(sequence) l_query, l_ref;

DMEM(scorev) E_spm;
DMEM(scorev) F_spm;
DMEM(scorev) H_spm;
DMEM(scorev) H_prev_spm;

#ifndef GRAIN_SCALE
#define GRAIN_SCALE 8
#endif

template <typename T>
using gref = bsg_global_pointer::reference<T>;
using guard = bsg_global_pointer::pod_address_guard;

#define TRACE
#ifdef TRACE
#define trace(x) \
    bsg_print_int(x)
#else
#define trace(x)
#endif

// DMEM(scorev) E_spm;
// DMEM(scorev) F_spm;
// DMEM(scorev) H_spm;
// DMEM(scorev) H_prev_spm;

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

inline void align
#ifdef CELLO_GLOBAL_STEALING
(sequence& seqa, sequence& seqb, gref<score> output) {
#else
(sequence& seqa, sequence& seqb, score& output) {
#endif
  int score_temp = 0;
/*
  for (int j = 1; j < SEQ_LEN; j++) {
    E_spm[j] = max(E_spm[j-1]-GAP_EXTEND, H_spm[j-1]-GAP_OPEN);
    F_spm[j] = 0;
    H_prev_spm[j] = H_spm[j];
    H_spm[j] = max(0, E_spm[j], F_spm[j]);
    if (H_spm[j] > score_temp) {
      score_temp = H_spm[j];
    }
  }
*/
  for (int i = 1; i < SEQ_LEN; i++) {
    for (int j = 1; j < SEQ_LEN; j++) {
      E_spm[j] = max(E_spm[j-1] - GAP_EXTEND,
                     H_spm[j-1] - GAP_OPEN);
      F_spm[j] = max(F_spm[j] - GAP_EXTEND,
                     H_spm[j] - GAP_OPEN);
      H_prev_spm[j] = H_spm[j];
      H_spm[j] = max(0,
                     E_spm[j],
                     F_spm[j],
                     H_prev_spm[j-1] + ((seqa[i] == seqb[j]) ? MATCH_SCORE : MISMATCH_SCORE));
      if (H_spm[j] > score_temp) {
        score_temp = H_spm[j];
      }
      // bsg_printf
      //     ("seqa[i=%2d]       = %2d\n"
      //      "seqb[j=%2d]       = %2d\n"
      //      "E_spm[j=%2d]      = %2d\n"
      //      "F_spm[j=%2d]      = %2d\n"
      //      "H_spm[j=%2d]      = %2d\n"
      //      "H_prev_spm[j=%2d] = %2d\n"
      //      "score_temp        = %2d\n"
      //      "---------------------------------\n",
      //      i, seqa[i],
      //      j, seqb[j],
      //      j, E_spm[j],
      //      j, F_spm[j],
      //      j, H_spm[j],
      //      j, H_prev_spm[j],
      //      score_temp);
    }
  }

  // Write result;
  output = score_temp;
}

int  cello_main(int argc, char *argv[])
{
    int grain = query.local_size()/(cello::threads()*GRAIN_SCALE);
    if (grain < 1)
        grain = 1;

#ifdef CELLO_GLOBAL_STEALING
    query.foreach_unrestricted(grain, [](int i, gref<sequence> dram_query) {
        gref<sequence> dram_ref = ref.at(i);
        gref<score> dram_output = output.at(i);
#else
    query.foreach(grain, [](int i, sequence &dram_query) {
        sequence& dram_ref= ref.local(i);
        score& dram_output = output.local(i);
#endif
        l_query = dram_query;
        l_ref = dram_ref;
        for (int j = 0; j < SEQ_LEN; j++) {
            E_spm[j] = 0;
            F_spm[j] = 0;
            H_spm[j] = 0;
            H_prev_spm[j] = 0;
        }
        align(l_query, l_ref, dram_output);
        trace(i);
    });
    return 0;
}
