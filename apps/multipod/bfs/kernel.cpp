//#include <atomic>
#include <algorithm>
#include "bsg_manycore.h"
#include "bsg_manycore.hpp"
#include "bsg_manycore_atomic.h"
#include "bsg_cuda_lite_barrier.h"
#include "bsg_barrier_multipod.h"

#define REV_WORK_SIZE 1

// FWD queue;
__attribute__((section(".dram"))) int g_q;

// DRAM variables to do edge parallel;
#define EP_QUEUE_SIZE 4096
__attribute__((section(".dram"))) int g_q2;
__attribute__((section(".dram"))) int g_edge_start[EP_QUEUE_SIZE];
__attribute__((section(".dram"))) int g_edge_stop[EP_QUEUE_SIZE];


// set a bit in dense bit vector;
inline void insert_into_dense(int id, int *dense_frontier)
{
    int word_idx = id/32;
    int bit_idx = id%32;
    int w = (1<<bit_idx);
    bsg_amoor(&dense_frontier[word_idx], w);
}

// multi-pod barrier; pod-row only;
// Used for synchronizing kernel start time between pods;
volatile int done[NUM_POD_X] = {0};
int alert = 0;

// Local storage for rev-dist;
int rev_dist[8];


// Rev loop;
inline void rev_loop(int v, int* distance, int* offsets, int *nonzeros, int *next_dense_frontier)
{
        if (distance[v] == -1) {
          int nz_start = offsets[v];
          int nz_stop = offsets[v+1];
          int nz = nz_start;

          // unroll by 8
          for (; nz+8 <= nz_stop; nz+=8) {
            int rev_src0 = nonzeros[nz+0];
            int rev_src1 = nonzeros[nz+1];
            int rev_src2 = nonzeros[nz+2];
            int rev_src3 = nonzeros[nz+3];
            int rev_src4 = nonzeros[nz+4];
            int rev_src5 = nonzeros[nz+5];
            int rev_src6 = nonzeros[nz+6];
            int rev_src7 = nonzeros[nz+7];
            asm volatile("": : :"memory");
            int dist_temp0 = distance[rev_src0];
            int dist_temp1 = distance[rev_src1];
            int dist_temp2 = distance[rev_src2];
            int dist_temp3 = distance[rev_src3];
            int dist_temp4 = distance[rev_src4];
            int dist_temp5 = distance[rev_src5];
            int dist_temp6 = distance[rev_src6];
            int dist_temp7 = distance[rev_src7];
            asm volatile("": : :"memory");
            rev_dist[0] = dist_temp0;
            rev_dist[1] = dist_temp1;
            rev_dist[2] = dist_temp2;
            rev_dist[3] = dist_temp3;
            rev_dist[4] = dist_temp4;
            rev_dist[5] = dist_temp5;
            rev_dist[6] = dist_temp6;
            rev_dist[7] = dist_temp7;
            asm volatile("": : :"memory");
            for (int i = 0; i < 8; i++) {
              if (rev_dist[i] != -1) {
                insert_into_dense(v, next_dense_frontier);
                return;
              }
            }
          }

          // unroll by 4
          for (; nz+4 <= nz_stop; nz+=4) {
            int rev_src0 = nonzeros[nz+0];
            int rev_src1 = nonzeros[nz+1];
            int rev_src2 = nonzeros[nz+2];
            int rev_src3 = nonzeros[nz+3];
            asm volatile("": : :"memory");
            int dist_temp0 = distance[rev_src0];
            int dist_temp1 = distance[rev_src1];
            int dist_temp2 = distance[rev_src2];
            int dist_temp3 = distance[rev_src3];
            asm volatile("": : :"memory");
            rev_dist[0] = dist_temp0;
            rev_dist[1] = dist_temp1;
            rev_dist[2] = dist_temp2;
            rev_dist[3] = dist_temp3;
            asm volatile("": : :"memory");
            for (int i = 0; i < 4; i++) {
              if (rev_dist[i] != -1) {
                insert_into_dense(v, next_dense_frontier);
                return;
              }
            }
          }

          // unroll by 1
          for (; nz < nz_stop; nz++) {
            int src0 = nonzeros[nz];
            if (distance[src0] != -1) {
              insert_into_dense(v, next_dense_frontier);
              return;
            }
          }
        }
}

// Kernel main;
extern "C" int kernel(
  int pod_id,
  int V,
  int direction,  // rev=1, fwd=0
  int* offsets,
  int* nonzeros,
  int* distance,
  int* curr_frontier,
  int frontier_size,
  int* next_dense_frontier
)
{
  // Initialize;
  bsg_barrier_hw_tile_group_init();
  //bsg_barrier_hw_tile_group_sync();
  bsg_barrier_multipod(pod_id-BASE_POD_ID, NUM_POD_X, done, &alert);

  // local variables;

  // kernel start;

  if (direction) {
    // REV  
    // Iterate through unvisited;
    int V_per_pod = (V+NUMPODS-1) / NUMPODS;
    int v_start = std::min(V, pod_id*V_per_pod);
    int v_end   = std::min(V, v_start+V_per_pod);

    if (__bsg_id == 0) {
      g_q = v_start;
      //bsg_print_int(v_start);
      //bsg_print_int(v_end);
      bsg_fence();
    }
    bsg_barrier_hw_tile_group_sync();
    bsg_cuda_print_stat_kernel_start();

    for (int v_start = bsg_amoadd(&g_q, REV_WORK_SIZE); v_start < v_end; v_start = bsg_amoadd(&g_q,REV_WORK_SIZE)) {
      int v = v_start;
      int stop = std::min(v_end, v_start+REV_WORK_SIZE);

      for (; v < stop; v++) {
        rev_loop(v, distance, offsets, nonzeros, next_dense_frontier);
      }
    }


  } else {
    // FWD
    int frontier_per_pod = (frontier_size+NUMPODS-1) / NUMPODS;
    int f_start = std::min(frontier_size, pod_id*frontier_per_pod);
    int f_end   = std::min(frontier_size, f_start+frontier_per_pod);
    int do_edge_parallel = DO_EDGE_PARALLEL;
    if (__bsg_id == 0) {
      g_q = f_start;
      g_q2 = 0;
      bsg_fence();
    }

    #ifdef WARM_FRONTIER
    for (int i = f_start+(__bsg_id*CACHE_BLOCK_WORDS); i < f_end; i+=bsg_tiles_X*bsg_tiles_Y*CACHE_BLOCK_WORDS) {
      asm volatile ("lw x0, %[p]" :: [p] "m" (curr_frontier[i]));
    }
    bsg_fence();
    #endif
    bsg_barrier_hw_tile_group_sync();
    bsg_cuda_print_stat_kernel_start();
    
    #ifdef FWD_QUEUE_STATIC
    for (int idx = f_start+__bsg_id; idx < f_end; idx+=bsg_tiles_X*bsg_tiles_Y) {
    #else
    for (int idx = bsg_amoadd(&g_q,1); idx < f_end; idx = bsg_amoadd(&g_q,1)) {
    #endif
      int src = curr_frontier[idx];
      int nz_start = offsets[src];
      int nz_stop = offsets[src+1];
      int nz_len = nz_stop - nz_start;
      if (do_edge_parallel && (nz_len > 128)) {
        int next_idx = bsg_amoadd(&g_q2,1);
        g_edge_start[next_idx] = nz_start;
        g_edge_stop[next_idx] = nz_stop;
      } else {
        // Forward traversal (vertex parallel)
        int nz = nz_start;
        for (; nz < nz_stop; nz++) {
          int dst = nonzeros[nz];
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
          int dst = nonzeros[nz];
          if (distance[dst] == -1) {
            insert_into_dense(dst, next_dense_frontier);
          }
        }
      }
    }
  }


  // Finish
  bsg_fence();
  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();

  return 0;
}
