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


extern "C"
int __attribute__ ((noinline)) pagerank_pull_u8(int bsg_attr_remote * bsg_attr_noalias in_indices,
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
      register float temp_new = 0.0f;
      register float temp_old = old_rank[d];
      register float error = 0.0f;
      register int first = in_indices[d];
      register int last = in_indices[d+1];
      register int od = out_degree[d];
      int s = first;
      for(;s + 8 < last; s += 8) {
              // Unrolled by hand so that loads end up consecutive For
              // some reason they do not. Register allocation only
              // works in GCC and is not necessary.
              register int idx0 asm ("s8");
              register int idx1 asm ("s9");
              register int idx2 asm ("s10");
              register int idx3 asm ("s11");
              register int idx4 asm ("t3");
              register int idx5 asm ("t4");
              register int idx6 asm ("t5");
              register int idx7 asm ("t6");
              idx0 = in_neighbors[s + 0];
              idx1 = in_neighbors[s + 1];
              idx2 = in_neighbors[s + 2];
              idx3 = in_neighbors[s + 3];
              idx4 = in_neighbors[s + 4];
              idx5 = in_neighbors[s + 5];
              idx6 = in_neighbors[s + 6];
              idx7 = in_neighbors[s + 7];
              asm volatile ("" ::: "memory");
              temp_new += contrib[idx0];
              temp_new += contrib[idx1];
              temp_new += contrib[idx2];
              temp_new += contrib[idx3];
              temp_new += contrib[idx4];
              temp_new += contrib[idx5];
              temp_new += contrib[idx6];
              temp_new += contrib[idx7];
      }

      for(; s < last; ++s){
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

