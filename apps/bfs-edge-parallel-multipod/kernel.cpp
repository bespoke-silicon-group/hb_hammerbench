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

// multi-pod barrier; pod-row only;
// Used for synchronizing kernel start time between pods;
volatile int done[NUM_POD_X] = {0};
int alert = 0;

static inline void barrier_multipod(int pod_id, volatile int* done, int* alert) {
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();
  
  // my pod id;
  int my_pod_x = pod_id - BASE_POD_ID; 

  // center tile;
  if ((__bsg_x == bsg_tiles_X/2) && (__bsg_y == bsg_tiles_Y/2)) {
    bsg_print_int(my_pod_x);

    // send remote store to the remote done list;
    volatile int* remote_done_ptr =  bsg_global_ptr((BSG_MACHINE_GLOBAL_X+(bsg_tiles_X/2)),
                                                    (BSG_MACHINE_GLOBAL_Y+(bsg_tiles_Y/2)),
                                                    &done[my_pod_x]);
    *remote_done_ptr  = 1;
    bsg_fence();

    if (my_pod_x == 0) {
      // main pod;
      // wait for everyone to be done;
      int done_count = 0;
      while (done_count < NUM_POD_X) {
        if (done[done_count] == 0) {
          continue;
        } else {
          done_count++;
        }
      }

      // once everyone joined, wake up everyone;
      for (int px = 0; px < NUM_POD_X; px++) {
        volatile int* remote_alert_ptr =  bsg_global_ptr((((1+px)*BSG_MACHINE_GLOBAL_X)+(bsg_tiles_X/2)),
                                                         (BSG_MACHINE_GLOBAL_Y+(bsg_tiles_Y/2)),
                                                         (alert));
        *remote_alert_ptr = 1;
      }
    } else {
      // other pods sleep;
      int tmp;
      while (1) {
        tmp = bsg_lr(alert);
        if (tmp) {
          break;
        } else {
          tmp = bsg_lr_aq(alert);
          if (tmp) {
            break;
          }
        }
      }
    } 
  }
  bsg_print_int(2);
  
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();
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
  barrier_multipod(pod_id, done, &alert);
  bsg_print_int(3);

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

    for (int v_start = bsg_amoadd(&g_q, BLOCK_SIZE); v_start < v_end; v_start = bsg_amoadd(&g_q,BLOCK_SIZE)) {
      int v = v_start;
      int stop = std::min(v_end, v_start+BLOCK_SIZE);

      // unroll by 2;
      #define REV_UNROLL 2
      for (;v+REV_UNROLL <= stop; v+=REV_UNROLL) {
        register int l_distance[REV_UNROLL];
        register int l_rev_offsets[REV_UNROLL+1];
        l_distance[0] = distance[v];
        l_distance[1] = distance[v+1];
        asm volatile("": : :"memory");
        l_rev_offsets[0] = offsets[v];
        l_rev_offsets[1] = offsets[v+1];
        l_rev_offsets[2] = offsets[v+2];
        asm volatile("": : :"memory");
      
        // 0
        if (l_distance[0] == -1) {
          int nz_start = l_rev_offsets[0];
          int nz_stop  = l_rev_offsets[1];
          for (int nz = nz_start; nz < nz_stop; nz++) {
            int src = nonzeros[nz];
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
            int src = nonzeros[nz];
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
          int nz_start = offsets[v];
          int nz_stop = offsets[v+1];
          for (int nz = nz_start; nz < nz_stop; nz++) {
            int src0 = nonzeros[nz];
            if (distance[src0] != -1) {
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
    int do_edge_parallel = (frontier_size < 128) && DO_EDGE_PARALLEL;
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
