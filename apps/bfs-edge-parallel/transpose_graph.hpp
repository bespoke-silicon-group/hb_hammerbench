#pragma once
#include <vector>
int transpose_graph(int V,
                    int E,
                    const std::vector<int>&fwd_offsets,
                    const std::vector<int>&fwd_nonzeros,
                    /* outputs */
                    std::vector<int>&rev_offsets,
                    std::vector<int>&rev_nonzeros);
