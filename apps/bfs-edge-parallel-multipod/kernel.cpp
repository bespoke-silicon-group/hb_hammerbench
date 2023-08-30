//#include <atomic>
#include <algorithm>
#include "bsg_manycore.h"
#include "bsg_manycore.hpp"
#include "bsg_manycore_atomic.h"
#include "bsg_cuda_lite_barrier.h"


#define BLOCK_SIZE 16

// FWD queue;
__attribute__((section(".dram"))) int g_q;

// DRAM variables to do edge parallel;
__attribute__((section(".dram"))) int g_q2;
__attribute__((section(".dram"))) int g_edge_start[128];
__attribute__((section(".dram"))) int g_edge_stop[128];


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
  int pod_id,
  int V,
  int direction,  // rev=1, fwd=0
  int* fwd_offsets,
  int* fwd_nonzeros,
  int* rev_offsets,
  int* rev_nonzeros,
  int* distance,
  int* curr_frontier,
  int frontier_size,
  int* next_dense_frontier
)
{
  // Initialize;
  bsg_barrier_hw_tile_group_init();
  bsg_barrier_hw_tile_group_sync();

  // local variables;
  int start;
  bsg_barrier_hw_tile_group_sync();

  // kernel start;
  bsg_cuda_print_stat_kernel_start();

  if (direction) {
    // REV  
    // Iterate through unvisited;
    int V_per_pod = (V+NUMPODS-1) / NUMPODS;
    int v_start = std::min(V, pod_id*V_per_pod);
    int v_end   = std::min(V, v_start+V_per_pod);

    if (__bsg_id == 0) {
      g_q = v_start;
      bsg_fence();
    }
    bsg_barrier_hw_tile_group_sync();

    for (int v_start = bsg_amoadd(&g_q, BLOCK_SIZE); v_start < v_end; v_start = bsg_amoadd(&g_q,BLOCK_SIZE)) {
      int v = v_start;
      int stop = std::min(V, v_start+BLOCK_SIZE);

      // unroll by 2;
      #define REV_UNROLL 2
      for (;v+REV_UNROLL <= stop; v+=REV_UNROLL) {
        register int l_distance[REV_UNROLL];
        register int l_rev_offsets[REV_UNROLL+1];
        l_distance[0] = distance[v];
        l_distance[1] = distance[v+1];
        asm volatile("": : :"memory");
        l_rev_offsets[0] = rev_offsets[v];
        l_rev_offsets[1] = rev_offsets[v+1];
        l_rev_offsets[2] = rev_offsets[v+2];
        asm volatile("": : :"memory");
      
        // 0
        if (l_distance[0] == -1) {
          int nz_start = l_rev_offsets[0];
          int nz_stop  = l_rev_offsets[1];
          for (int nz = nz_start; nz < nz_stop; nz++) {
            int src = rev_nonzeros[nz];
            if (distance[src] != -1) {
              insert_into_dense(v, next_dense_frontier);
              break;
            } 
          }
        }

        // 1
        if (l_distance[1] == -1) {
          int nz_start = l_rev_offsets[1];
          int nz_stop  = l_rev_offsets[2];
          for (int nz = nz_start; nz < nz_stop; nz++) {
            int src = rev_nonzeros[nz];
            if (distance[src] != -1) {
              insert_into_dense(v+1, next_dense_frontier);
              break;
            } 
          }
        }
      }

      // Unroll by 1
      for (; v < stop; v++) {
        if (distance[v] == -1) {
          int nz_start = rev_offsets[v];
          int nz_stop = rev_offsets[v+1];
          for (int nz = nz_start; nz < nz_stop; nz++) {
            int src0 = rev_nonzeros[nz];
            if (distance[src] != -1) {
              insert_into_dense(v, next_dense_frontier);
              break;
            }
          }
        }
      }
    }

  } else {
    // FWD
    int frontier_per_pod = (frontier_size+NUMPODS-1) / NUMPODS;
    int f_start = std::min(frontier_size, pod_id*frontier_per_pod);
    int f_end   = std::min(frontier_size, f_start+frontier_per_pod);
    int do_edge_parallel = frontier_size < 128;
    if (__bsg_id == 0) {
      g_q = f_start;
      g_q2 = 0;
      bsg_fence();
    }
    bsg_barrier_hw_tile_group_sync();
    
    for (int idx = bsg_amoadd(&g_q,1); idx < f_end; idx = bsg_amoadd(&g_q,1)) {
      int src = curr_frontier[idx];
      int nz_start = fwd_offsets[src];
      int nz_stop = fwd_offsets[src+1];
      int nz_len = nz_stop - nz_start;
      if (do_edge_parallel && (nz_len > 128)) {
        int next_idx = bsg_amoadd(&g_q2,1);
        g_edge_start[next_idx] = nz_start;
        g_edge_stop[next_idx] = nz_stop;
      } else {
        // Forward traversal (vertex parallel)
        int nz = nz_start;
        for (; nz < nz_stop; nz++) {
          int dst = fwd_nonzeros[nz];
          if (distance[dst] == -1) {
            insert_into_dense(dst, next_dense_frontier);
          }
        }
      }
    }

    bsg_fence();
    bsg_barrier_hw_tile_group_sync();

    // Edge parallel
    if (do_edge_parallel) {
      int l_q2 = g_q2;
      for (int idx = 0; idx < l_q2; idx++) {
        int nz_start = g_edge_start[idx];
        int nz_stop = g_edge_stop[idx];
        for (int nz = nz_start+__bsg_id; nz < nz_stop; nz += bsg_tiles_X*bsg_tiles_Y) {
          int dst = fwd_nonzeros[nz];
          if (distance[dst] == -1) {
            insert_into_dense(dst, next_dense_frontier);
          }
        }
      }
    }
  }


  // kernel end
  bsg_fence();
  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();

}
