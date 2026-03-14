#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <x86intrin.h>

#define DEBUG 0

// pretty-print a 2D float matrix
void print_matrix(float** matrix, int n, int m) {
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      printf("%f ", matrix[i][j]);
    }
    printf("\n");
  }
}

// pretty-print the output of a bellman-ford run
void print_bellman_ford(float* dist, int num_vertices, int start) {
  if (dist == NULL) {
    printf("No shortest paths--negative cycle detected!\n");
  } else {
    for (int i = 0; i < num_vertices; i++) {
      printf("%d -> %d: %f\n", start, i, dist[i]);
    }
  }
}

// parse a graph from a file in matrix market format
// returns the graph as an adjacency matrix of incoming edges
// ownership of allocated heap space of the matrix is passed to the caller
// num_vertices is a return parameter
float** parse_graph(char* filename, int* num_vertices) {
  // open the file
  FILE* fptr = fopen(filename, "r");
  if (fptr == NULL) {
    return NULL;
  }

  // we know this is matrix market format
  // so we should read the third line

  // line buffer
  char buf[1024];

  fgets(buf, sizeof(buf), fptr); // 1st (throw away)
  fgets(buf, sizeof(buf), fptr); // 2nd (throw away)
  fgets(buf, sizeof(buf), fptr); // 3rd (important--matrix dimensions)

  // read in dimensions
  int n, m, e;
  if (sscanf(buf, "%d %d %d", &n, &m, &e) < 3) {
    perror("cannot read matrix dimensions");
    fclose(fptr);
    return NULL;
  }

  // allocate space for the matrix
  float** matrix = (float**) malloc(n *  sizeof(float*));
  if (matrix == NULL) {
    perror("malloc");
    fclose(fptr);
    return NULL;
  }
  for (int i = 0; i < n; i++) {
    matrix[i] = (float*) malloc(m * sizeof(float));
    if (matrix[i] == NULL) {
      perror("malloc");
      fclose(fptr);
      return NULL;
    }
    for (int j = 0; j < m; j++) {
      matrix[i][j] = INFINITY;
    }
  }

  // write in the edges as an adjacency matrix
  int v1, v2;
  float weight;
  while (fgets(buf, sizeof(buf), fptr) != NULL) {
    if (sscanf(buf, "%d %d %f", &v1, &v2, &weight) < 3) {
      perror("bad matrix line\n");
      free(matrix);
      fclose(fptr);
      return NULL;
    }
    matrix[v1 -1][v2 - 1] = weight;
  }

  // we're done!
  *num_vertices = n;
  fclose(fptr);
  return (void*) matrix;
}

float* flatten_graph(float** graph, int V)
{
  float* flat_graph = (float*) malloc(V*V*sizeof(float));
  for (int i = 0; i < V; i++) {
    for (int j = 0; j < V; j++) {
      flat_graph[i*V + j] = graph[i][j];
    }
  }
  return flat_graph;
}

// computes the shortest paths from a specified starting vertex
// matrix is an adjacency matrix of incoming edges
// num_vertices is the number of vertices in the graph
// start is an index of the desired starting vertex
// returns the shortest paths memoization array if no negative cycles are detected,
// and returns NULL if a negative cycle was detected.
// ownership of the returned memoization array is passed to the caller.
// based on pseudocode: https://web.stanford.edu/class/archive/cs/cs161/cs161.1168/lecture14.pdf
float* bellman_ford(float* matrix, int num_vertices, int start) {
  // allocate and initialize memoization array
  float* dist = (float*) malloc(num_vertices * sizeof(float));
  for (int i = 0; i < num_vertices; i++) {
    dist[i] = INFINITY;
  }
  dist[start] = 0;

  unsigned long before, after;
  before = _rdtsc();
  // start of the algorithm
  for (int iter = 0; iter < num_vertices - 1; iter++) { // do this V - 1 times
    for (int i = 0; i < num_vertices; i++) {
      for (int j = 0; j < num_vertices; j++) {
        float new_dist = dist[j] + matrix[i * num_vertices + j];
        if (new_dist < dist[i]) {
          dist[i] = new_dist; // relax edges
        }
      }
    }
  }
  after = _rdtsc();
  printf("Cycles: %lu\n", after - before);
  return dist;
}

int main (int argc, char** argv) {
  char* filename = argv[1];
  int num_vertices = -1;

  float** matrix = parse_graph(filename, &num_vertices);
  if (num_vertices == -1) {
    printf("Did not get valid matrix!\n");
    return 1;
  }

  if (DEBUG) {
    printf("MATRIX WITH %d VERTICES:\n", num_vertices);
    print_matrix(matrix, num_vertices, num_vertices);
    printf("\n");
  }

  int start = 0;
  if (DEBUG) { printf("SHORTEST PATHS FROM VERTEX %d\n", start); }
  float* paths = bellman_ford(flatten_graph(matrix, num_vertices), num_vertices, start);
  if (DEBUG) { print_bellman_ford(paths, num_vertices, start); }
  return 0;
}