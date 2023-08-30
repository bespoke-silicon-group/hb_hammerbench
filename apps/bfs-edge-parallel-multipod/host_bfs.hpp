#pragma once
#include <vector>
void host_bfs(
    int root,
    int V,
    const std::vector<int>&fwd_offsets,
    const std::vector<int>&fwd_nonzeros,
    const std::vector<int>&rev_offsets,
    const std::vector<int>&rev_nonzeros,
    std::vector<int> &distance,
    std::vector<int> &direction
);
