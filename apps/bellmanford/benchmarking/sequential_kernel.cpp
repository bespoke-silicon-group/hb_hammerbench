//#include <atomic>
#include <algorithm>
#include <cmath>
#include "bsg_manycore.h"
#include "bsg_manycore.hpp"
#include "bsg_manycore_atomic.h"
#include "bsg_cuda_lite_barrier.h"
#include "bsg_barrier_multipod.h"
#include "bsg_striped_array.hpp"
#include "bsg_tile_group_barrier.hpp"
#include "bsg_set_tile_x_y.h"

using namespace bsg_manycore;

// DEBUG: printing functions for arrays and matrices using bsg_printf
void print_array(float* array, int dim, char* name) {
  char buf[1024];
  int offset = 0;
  while (name[offset]) {
    buf[offset] = name[offset];
    offset++;
  }
  
  for (int v = 0; v < dim; v++) {
    int dist = (int)array[v];
    if (array[v] == INFINITY) {
      buf[offset++] = 'i';
      buf[offset++] = 'n';
      buf[offset++] = 'f';
    } else {
      // convert dist to string and append to buf
      char dist_buf[16];
      int dist_offset = 0;
      if (dist == 0) {
        dist_buf[dist_offset++] = '0';
      } else {
        while (dist > 0 && dist_offset < 16) {
          int digit = dist % 10;
          dist_buf[dist_offset++] = '0' + digit;
          dist /= 10;
        }
      }
      // reverse dist_buf into buf
      for (int i = dist_offset - 1; i >= 0; i--) {
        buf[offset++] = dist_buf[i];
      }
    }
    if (v < dim - 1) {
        buf[offset++] = ',';
        buf[offset++] = ' ';
    }
  }

  char final_buf[offset + 2];
  for (int i = 0; i < offset; i++) {
    final_buf[i] = buf[i];
  }
  final_buf[offset++] = '\n';
  final_buf[offset] = '\0';
  
  bsg_printf("%s", final_buf);
}

void print_graph(float* graph, int dim) {
  char row[4] = {'0', ':', ' ', '\0'};
  for(int i = 0; i < dim; i++) {
    print_array(graph + (i*dim), dim, row);
    row[0]++;
  }
}

extern "C" int kernel(
    int pod_id,
    int dim,
    float* graph,
    int start,
    int tile_group_dim_x,
    int tile_group_dim_y,
    int num_groups,
    float* distance
)
{
  // Tile-group barrier init
  bsg_barrier_tile_group_init();
  bsg_barrier_tile_group_sync();
  bsg_cuda_print_stat_kernel_start();

  #define UNROLL 8
  #define REMAINDER (dim % UNROLL)

  float js[UNROLL];
  float graph_js[UNROLL];

  // Bellman-Ford
  for (int iter = 0; iter < dim - 1; iter++) { // do this V - 1 times
    for (int i = 0; i < dim; i++) {
      register float distance_i = distance[i];
      for (int j = 0; j < ((dim / UNROLL) * UNROLL); j+= UNROLL) {
        js[0] = distance[j];
        js[1] = distance[j+1];
        js[2] = distance[j+2];
        js[3] = distance[j+3];
        js[4] = distance[j+4];
        js[5] = distance[j+5];
        js[6] = distance[j+6];
        js[7] = distance[j+7];
        graph_js[0] = graph[i * dim + j];
        graph_js[1] = graph[i * dim + j + 1];
        graph_js[2] = graph[i * dim + j + 2];
        graph_js[3] = graph[i * dim + j + 3];
        graph_js[4] = graph[i * dim + j + 4];
        graph_js[5] = graph[i * dim + j + 5];
        graph_js[6] = graph[i * dim + j + 6];
        graph_js[7] = graph[i * dim + j + 7];
        asm volatile("": : :"memory");
        bsg_fence();
        float min = js[0] + graph_js[0];
        for (int k = 1; k < UNROLL; k++) {
          if ((js[k] + graph_js[k]) < min) {
            min = js[k] + graph_js[k];
          }
        }
        if (min < distance_i) {
          distance[i] = min;
          distance_i = min;
          bsg_fence();
        }
      }
      for (int j = ((dim / UNROLL) * UNROLL); j < dim; j++) {
        float dist = distance[j];
        float graph_j = graph[i * dim + j];
        bsg_fence();
        if (dist + graph_j < distance_i) {
          distance[i] = dist + graph_j;
          distance_i = dist + graph_j;
          bsg_fence();
        }
      }
    }
  }
  bsg_cuda_print_stat_kernel_end();
  return 0;
}
