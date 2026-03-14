#include "host_bellman_ford.hpp"
#include <set>
#include <cstdio>
#include <cmath>

bool host_bellman_ford(
    int root,
    int V,
    float** matrix,
    std::vector<float> &dist
) {
    dist = std::vector<float>(V, INFINITY);
    dist[root] = 0;

  // start of the algorithm
  for (int iter = 0; iter < V - 1; iter++) { // do this V - 1 times
    for (int i = 0; i < V; i++) {
      for (int j = 0; j < V; j++) {
        if (matrix[i][j] != NAN) { // loop over all existing edges
          dist[i] = fminf(dist[i], dist[j] + matrix[i][j]); // relax edges
        }
      }
    }
  }

  // detect negative cycles
  for (int i = 0; i < V; i++) {
    for (int j = 0; j < V; j++) {
      if (matrix[i][j] != NAN) { // loop over all existing edges
        if (dist[i] > dist[j] + matrix[i][j]) {
          return true;
        }
      }
    }
  }
  return false;
}
