#include <algorithm>
#include "bsg_manycore.h"
#include "bsg_manycore.hpp"
#include "bsg_manycore_atomic.h"
#include "bsg_cuda_lite_barrier.h"
#include "tg_amoadd_barrier.h"

#define BLOCK_SIZE 16
#define NUM_TILES_PER_TG (bsg_tiles_X*bsg_tiles_Y/NUM_TG)
#define NUM_QUERY 8

// FWD queue;
__attribute__((section(".dram"))) int g_q[NUM_TG];

// tg amoadd barrier;
__attribute__((section(".dram"))) int tg_lock[NUM_TG];
int sense;

// tg variables;
int tgid;
int tid;
int *tg_distance;
int *tg_curr_frontier;
int *tg_next_dense_frontier;



// set a bit in dense bit vector;
inline void insert_into_dense(int id, int *dense_frontier)
{
    int word_idx = id/32;
    int bit_idx = id%32;
    int w = (1<<bit_idx);
    bsg_amoor(&dense_frontier[word_idx], w);
}


// Kernel main;
extern "C" int kernel(
  int* offsets,
  int* nonzeros,
  int* distance,
  int* curr_frontier,
  int* curr_frontier_offsets,
  int* next_dense_frontier,
  int next_dense_frontier_size
)
{
  // Initialize;
  bsg_barrier_hw_tile_group_init();
  bsg_barrier_hw_tile_group_sync();

  // tile group variable; setup amoadd barrier;
  tgid = __bsg_id / NUM_TILES_PER_TG;  
  tid = __bsg_id % NUM_TILES_PER_TG;  
  sense = 1;
  if (tid == 0) {
    tg_lock[tgid] = 0;
  }
  bsg_fence();

  // Kernel start;
  bsg_barrier_hw_tile_group_sync();
  bsg_cuda_print_stat_kernel_start();

  for (int task_id = tgid; task_id < NUM_QUERY; task_id += NUM_TG) {
    // current tg task;
    tg_distance = &distance[task_id*VERTEX];
    tg_curr_frontier = &curr_frontier[curr_frontier_offsets[task_id]];
    tg_next_dense_frontier = &next_dense_frontier[task_id*next_dense_frontier_size];
    int num_frontiers = curr_frontier_offsets[task_id+1] - curr_frontier_offsets[task_id];
    if (tid == 0) {
      g_q[tgid] = 0;
      bsg_print_int(task_id);
    }
    bsg_fence();
    tg_amoadd_barrier(&tg_lock[tgid], &sense, tgid*(8/NUM_TG), (8/NUM_TG));

    // main kernel;
    for (int idx = bsg_amoadd(&g_q[tgid],1); idx < num_frontiers; idx = bsg_amoadd(&g_q[tgid],1)) {
      int src = tg_curr_frontier[idx];
      int nz_start = offsets[src];
      int nz_stop = offsets[src+1];
      // Forward traversal (vertex parallel)
      for (int nz = nz_start; nz < nz_stop; nz++) {
        int dst = nonzeros[nz];
        if (tg_distance[dst] == -1) {
          insert_into_dense(dst, tg_next_dense_frontier);
        }
      }
    }
    bsg_fence();
    tg_amoadd_barrier(&tg_lock[tgid], &sense, tgid*(8/NUM_TG), (8/NUM_TG));
  }

  // Kernel End;
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();


  // Finish
  bsg_fence();
  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();

  return 0;
}
