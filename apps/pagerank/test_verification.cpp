#include "verification.hpp"
#include <cassert>
#include <cmath>
#include <limits>
#include <initializer_list>
#include <cstdio>

int main() {
  const float beta = (1.0f - 0.85f) / 103689.0f;
  for (float inverse : {1.0f, 0.5f, 1.0f/3.0f, 1.0f/7.0f}) {
    const float contribution = beta * inverse;
    assert(hb_pagerank_zero_indegree_matches(beta, contribution, beta, inverse));
    assert(!hb_pagerank_zero_indegree_matches(0.0f, 0.0f, beta, inverse));
    assert(!hb_pagerank_zero_indegree_matches(0.0f, contribution, beta, inverse));
    assert(!hb_pagerank_zero_indegree_matches(beta, 0.0f, beta, inverse));
    assert(!hb_pagerank_zero_indegree_matches(std::nextafter(beta, 1.0f), contribution,
                                            beta, inverse));
  }
  for (float invalid : {std::numeric_limits<float>::quiet_NaN(),
                        std::numeric_limits<float>::infinity(),
                        -std::numeric_limits<float>::infinity()}) {
    assert(!hb_pagerank_zero_indegree_matches(invalid, beta, beta, 1.0f));
    assert(!hb_pagerank_zero_indegree_matches(beta, invalid, beta, 1.0f));
    assert(!hb_pagerank_zero_indegree_matches(invalid, invalid, invalid, 1.0f));
    assert(!hb_pagerank_zero_indegree_matches(beta, invalid, beta, invalid));
  }
  std::puts("PageRank zero-indegree verifier tests passed");
}
