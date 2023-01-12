#define BSG_TILE_GROUP_X_DIM 16
#define BSG_TILE_GROUP_Y_DIM 8
#define bsg_tiles_X BSG_TILE_GROUP_X_DIM
#define bsg_tiles_Y BSG_TILE_GROUP_Y_DIM

#define GRANULARITY_PUSH 10
#include <bsg_manycore.h>
#include <bsg_tile_group_barrier.hpp>
#include <atomic>
#include <math.h>
#include <local_range.h>
#include <vertex_struct.h>
#include <cstring>
#include <numeric>
#include <array>

//#define DEBUG

#ifdef DEBUG
#define pr_dbg(fmt, ...)                        \
    do{ if (__bsg_id == 0) bsg_printf(fmt, ##__VA_ARGS__); } while (0)
#else
#define pr_dbg(fmt, ...)
#endif

//keep these:
__attribute__((section(".dram"))) int damp;
__attribute__((section(".dram"))) int beta_score;

__attribute__((section(".dram"))) std::atomic<int> workq;

extern "C"
int __attribute__ ((noinline)) pagerank_push(int bsg_attr_remote * bsg_attr_noalias out_indices,
                                             int bsg_attr_remote * bsg_attr_noalias out_neighbors,
                                             int bsg_attr_remote * bsg_attr_noalias out_degree,
                                             int bsg_attr_remote * bsg_attr_noalias old_rank,
                                             int bsg_attr_remote * bsg_attr_noalias new_rank,
                                             int bsg_attr_remote * bsg_attr_noalias contrib,
                                             int bsg_attr_remote * bsg_attr_noalias contrib_new,
                                             int V) {    
  bsg_barrier_hw_tile_group_init();
  bsg_cuda_print_stat_kernel_start();
  ////////////////////////////////////
  // update edge - push direction   //
  ////////////////////////////////////
  int start = 0;
  int end = V;
  int length = end - start;
  for(int id = workq.fetch_add(GRANULARITY_PUSH, std::memory_order_relaxed); id < length; id = workq.fetch_add(GRANULARITY_PUSH, std::memory_order_relaxed)) {
      int stop = (id + GRANULARITY_PUSH) > length ? length : (id + GRANULARITY_PUSH);
      for (int s = start + id; s < start + stop; s++) {
          // read your contribution
          register int s_contrib = contrib[s];
          register int first = out_indices[s];
          register int last  = out_indices[s+1];
          int d = first;
          for (; d < last; d++) {
              // add contribution to d            
              register int idx = out_neighbors[d];
              bsg_amoadd(&new_rank[idx], s_contrib);
          }        
      }
  }
  bsg_barrier_hw_tile_group_sync();
  ///////////////////
  // update vertex //
  ///////////////////
  for (int id = __bsg_id*STRIDE; id < V; id += bsg_tiles_X*bsg_tiles_Y*STRIDE) {
      // 1. set old rank and new rank
      int tmp = bsg_amoadd(&new_rank[id], beta_score);
      old_rank[id] = 0;
      // 2. compute new contribution
      new_contrib[id] = tmp / out_degree[id];
      // host will swap old_rank and new_rank
  }

  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();
  return 0;
}


extern "C"
int __attribute__ ((noinline)) pagerank_push_u8(int bsg_attr_remote * bsg_attr_noalias out_indices,
                                                int bsg_attr_remote * bsg_attr_noalias out_neighbors,
                                                int bsg_attr_remote * bsg_attr_noalias out_degree,
                                                int bsg_attr_remote * bsg_attr_noalias old_rank,
                                                int bsg_attr_remote * bsg_attr_noalias new_rank,
                                                int bsg_attr_remote * bsg_attr_noalias contrib,
                                                int bsg_attr_remote * bsg_attr_noalias contrib_new,
                                                int V) {
    
    return pagerank_push(
        out_indices,
        out_neighbors,
        out_degree,
        old_rank,
        new_rank,
        contrib,
        contrib_new
        );
}
