#ifndef HB_PAGERANK_VERIFICATION_HPP
#define HB_PAGERANK_VERIFICATION_HPP

#include "../common/host_sse.hpp"

// With no incoming edges, the sum is exactly zero: rank must equal beta,
// and contribution requires only one FP32 multiply, with no reduction error.
inline bool hb_pagerank_zero_indegree_matches(float rank, float contribution,
                                             float beta, float out_degree_inv) {
  const float expected_contribution = beta * out_degree_inv;
  return hb_sse_finite(rank) && hb_sse_finite(contribution) &&
         hb_sse_finite(beta) && hb_sse_finite(out_degree_inv) &&
         hb_sse_finite(expected_contribution) &&
         rank == beta && contribution == expected_contribution;
}

#endif
