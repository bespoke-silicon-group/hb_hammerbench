
#define BSG_TILE_GROUP_X_DIM 16
#define BSG_TILE_GROUP_Y_DIM 8
#define bsg_tiles_X BSG_TILE_GROUP_X_DIM
#define bsg_tiles_Y BSG_TILE_GROUP_Y_DIM

#define GRANULARITY_PULL 10
#include <bsg_manycore.h>
#include <bsg_tile_group_barrier.hpp>
#include <atomic>
#include <math.h>
#include <local_range.h>
#include <vertex_struct.h>
#include <cstring>

//#define DEBUG

#ifdef DEBUG
#define pr_dbg(fmt, ...)                        \
    do{ if (__bsg_id == 0) bsg_printf(fmt, ##__VA_ARGS__); } while (0)
#else
#define pr_dbg(fmt, ...)
#endif

//keep these:
__attribute__((section(".dram"))) float damp;
__attribute__((section(".dram"))) float beta_score;

__attribute__((section(".dram"))) std::atomic<int> workq;

extern "C"
int __attribute__ ((noinline)) pagerank_pull(int bsg_attr_remote * bsg_attr_noalias in_indices,
                                             int bsg_attr_remote * bsg_attr_noalias in_neighbors,
                                             int bsg_attr_remote * bsg_attr_noalias out_degree,
                                             float bsg_attr_remote * bsg_attr_noalias old_rank,
                                             float bsg_attr_remote * bsg_attr_noalias new_rank,
                                             float bsg_attr_remote * bsg_attr_noalias contrib,
                                             float bsg_attr_remote * bsg_attr_noalias contrib_new,
                                             int V) {
  bsg_barrier_hw_tile_group_init();
  bsg_cuda_print_stat_kernel_start();

  int start = 0;
  int end = V;
  int length = end - start;
  for(int id = workq.fetch_add(GRANULARITY_PULL, std::memory_order_relaxed); id < length; id = workq.fetch_add(GRANULARITY_PULL, std::memory_order_relaxed)) {
    int stop = (id + GRANULARITY_PULL) > length ? length : (id + GRANULARITY_PULL);
    for (int d = start + id; d < start + stop; d++) {
      register float temp_new = 0.0;
      register float temp_old = old_rank[d];
      register float error = 0.0;
      register int first = in_indices[d];
      register int last = in_indices[d+1];
      register int od = out_degree[d];
      int s = first;
      for(; s < last; s++) {
        register int idx = in_neighbors[s];
        register float tmp = contrib[idx];
        temp_new = temp_new + tmp;
      }

      temp_new = beta_score + damp * temp_new;
      error = fabs(temp_new - temp_old);
      old_rank[d] = temp_new;
      contrib_new[d] = temp_new / od;           
    }
  }

  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();
  return 0;
}


