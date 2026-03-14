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

#define NUM_TILE_GROUPS ((bsg_pod_X * bsg_pod_Y) / (bsg_tiles_X * bsg_tiles_Y))
#define TILE_GROUP_SIZE (bsg_tiles_X * bsg_tiles_Y)

#define SCRATCH_SIZE 4096 * 3 / 4
// one float + one int per (dist, src) = 8 bytes
#define BATCH ((SCRATCH_SIZE / 8 < VERTEX) ? SCRATCH_SIZE / 8 : VERTEX)

using TileGroupSharedFloatMem =
    VolatileTileGroupStripedArray<float, TILE_GROUP_SIZE * BATCH, bsg_tiles_X, bsg_tiles_Y, 1>;
using TileGroupSharedIntMem =
    VolatileTileGroupStripedArray<int, TILE_GROUP_SIZE * BATCH, bsg_tiles_X, bsg_tiles_Y, 1>;

// Tile-group shared scratch for reductions
TileGroupSharedFloatMem can_dist;
TileGroupSharedIntMem can_src;

// DRAM scratch for synchronous Bellman-Ford
__attribute__((section(".dram"))) float distance_next[VERTEX];

// Grid-wide barrier across tile groups (single pod)
__attribute__((section(".dram"))) volatile int grid_barrier_count = 0;
__attribute__((section(".dram"))) volatile int grid_barrier_sense = 0;

// Early-exit flag: set to 1 if any distance changes in an iteration
__attribute__((section(".dram"))) volatile int global_changed = 0;


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
  for (int i = 0; i < dim; i++) {
    print_array(graph + (i * dim), dim, row);
    row[0]++;
  }
}

// Helper function to sync across tile groups in a grid
static inline void grid_barrier_sync(int num_groups)
{
  bsg_barrier_tile_group_sync();

  if (__bsg_id == 0) {
    int my_sense = grid_barrier_sense ^ 1;
    int old = bsg_amoadd((int*)&grid_barrier_count, 1);

    if (old == num_groups - 1) {
      grid_barrier_count = 0;
      bsg_fence();
      grid_barrier_sense = my_sense;
      bsg_fence();
    } else {
      while (grid_barrier_sense != my_sense) { }
      bsg_fence();
    }
  }

  bsg_barrier_tile_group_sync();
}

extern "C" int kernel(
    int pod_id,
    float* graph,
    int start,
    float* distance
)
{
  bsg_barrier_tile_group_init();
  bsg_barrier_tile_group_sync();
  bsg_cuda_print_stat_kernel_start();

  int dim = VERTEX;

  // Balanced partition of vertices across tile groups
  int base_vertices = dim / NUM_TILE_GROUPS;
  int rem_vertices  = dim % NUM_TILE_GROUPS;

  int tg = __bsg_tile_group_id;
  int group_vertices_start = tg * base_vertices + (tg < rem_vertices ? tg : rem_vertices);
  int group_vertices_count = base_vertices + (tg < rem_vertices ? 1 : 0);
  int group_vertices_end   = group_vertices_start + group_vertices_count;

  // Balanced partition of candidate sources across tiles
  int base_candidates = dim / TILE_GROUP_SIZE;
  int rem_candidates  = dim % TILE_GROUP_SIZE;

  int tid = __bsg_id;
  int candidates_start = tid * base_candidates + (tid < rem_candidates ? tid : rem_candidates);
  int candidates_count = base_candidates + (tid < rem_candidates ? 1 : 0);
  int candidates_end   = candidates_start + candidates_count;

  for (int iter = 0; iter < dim - 1; iter++) {

    // 1) Compute next distances in batches
    for (int v = group_vertices_start; v < group_vertices_end; v += BATCH) {
      // Initialize this tile's local shard of the striped scratch
      for (int b = 0; b < BATCH; b++) {
        int v_b = v + b;
        if (v_b < group_vertices_end) {
          can_dist.at_local(b) = distance[v_b];
          can_src.at_local(b)  = -1;
        } else {
          can_dist.at_local(b) = INFINITY;
          can_src.at_local(b)  = -1;
        }
      }

      // Scan candidate sources assigned to this tile
      for (int c = candidates_start; c < candidates_end; c++) {
        float dist_to_c = distance[c];
        if (dist_to_c == INFINITY) {
          continue;
        }

        for (int b = 0; b < BATCH; b++) {
          int v_b = v + b;
          if (v_b >= group_vertices_end) {
            break;
          }

          float dist_to_v_from_c = graph[v_b * dim + c];
          if (dist_to_v_from_c == INFINITY) {
            continue;
          }

          float new_dist = dist_to_v_from_c + dist_to_c;
          float old_dist = can_dist.at_local(b);
          if (old_dist == INFINITY || new_dist < old_dist) {
            can_dist.at_local(b) = new_dist;
            can_src.at_local(b)  = c;
          }
        }
      }

      bsg_fence();
      bsg_barrier_tile_group_sync();

      // Tile 0 reduces each batch slot across the tile group
      if (__bsg_id == 0) {
        for (int b = 0; b < BATCH; b++) {
          int v_b = v + b;
          if (v_b >= group_vertices_end) {
            break;
          }

          int base = b * TILE_GROUP_SIZE;
          float global_dist = can_dist[base + 0];
          int   global_cand = can_src[base + 0];

          for (int t = 1; t < TILE_GROUP_SIZE; t++) {
            float dist = can_dist[base + t];
            int   cand = can_src[base + t];

            if (cand != -1 && (global_dist == INFINITY || dist < global_dist)) {
              global_dist = dist;
              global_cand = cand;
            }
          }

          distance_next[v_b] = global_dist;
        }
      }

      bsg_fence();
      bsg_barrier_tile_group_sync();
    }

    // 2) Ensure all groups finished writing distance_next[]
    grid_barrier_sync(NUM_TILE_GROUPS);

    // 3) Commit distance_next -> distance in parallel across the tile group
    for (int v = group_vertices_start + __bsg_id; v < group_vertices_end; v += TILE_GROUP_SIZE) {
      float dist_next = distance_next[v];
      float dist_cur  = distance[v];
      if (dist_next < dist_cur) {
        distance[v] = dist_next;
      }
    }

    // 4) Ensure all groups committed before next iteration reads distance[]
    grid_barrier_sync(NUM_TILE_GROUPS);
  }

  bsg_fence();
  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_tile_group_sync();

  return 0;
}