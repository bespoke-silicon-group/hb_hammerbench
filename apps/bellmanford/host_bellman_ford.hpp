#pragma once
#include <vector>
// computes the shortest paths from a specified starting vertex
// V is the number of vertices in the graph
// root is an index of the desired starting vertex
// matrix is an adjacency matrix of incoming edges
// distance is the memoization array to store the results
// returns whether a negative edge cycle was found
// based on pseudocode: https://web.stanford.edu/class/archive/cs/cs161/cs161.1168/lecture14.pdf
bool host_bellman_ford(
    int root,
    int V,
    float** matrix,
    std::vector<float> &distance
);
