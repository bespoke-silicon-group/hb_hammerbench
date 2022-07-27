
#define BSG_TILE_GROUP_X_DIM 16
#define BSG_TILE_GROUP_Y_DIM 8
#define bsg_tiles_X BSG_TILE_GROUP_X_DIM
#define bsg_tiles_Y BSG_TILE_GROUP_Y_DIM
#define CACHE_LINE 16
#define GRANULARITY_PULL 10
#define KERNEL_CURRENT_POD SIM_KERNEL_CURRENT_POD
#define KERNEL_CURRENT_BLOCK SIM_KERNEL_CURRENT_BLOCK
#define KERNEL_NUM_PODS 64
#include <bsg_manycore.h>
#include <bsg_tile_group_barrier.hpp>
bsg_barrier<bsg_tiles_X, bsg_tiles_Y> barrier;
#include <atomic>
#include <pr_pull.hpp>
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

int edgeset_apply_pull_serial(int *in_indices , int *in_neighbors, float * new_rank, float * contrib, int V)
{
  bsg_barrier_hw_tile_group_init();
  int tag = 2; 
  bsg_cuda_print_stat_start(tag);
//  pr_dbg("Enter the edge pull kernel, dcsr_length is: %d\n", dcsr_length);
  int rows_within_pod = (V - KERNEL_CURRENT_POD) % KERNEL_NUM_PODS == 0 ? 
                        ((V - KERNEL_CURRENT_POD) / KERNEL_NUM_PODS) : ((V - KERNEL_CURRENT_POD) / KERNEL_NUM_PODS + 1);
//  bsg_printf("Rows within block is %d\n", rows_within_pod);
//    for (int id = start + __bsg_id; id < end; id = id + bsg_tiles_X * bsg_tiles_Y) {
  for(int id = workq.fetch_add(GRANULARITY_PULL, std::memory_order_relaxed); id < rows_within_pod; id = workq.fetch_add(GRANULARITY_PULL, std::memory_order_relaxed)) {
    int stop = (id + GRANULARITY_PULL) > rows_within_pod ? rows_within_pod : (id + GRANULARITY_PULL);
    for (int d = id; d < stop; d++) {
      int row = KERNEL_CURRENT_POD + d * KERNEL_NUM_PODS;
//      bsg_printf("Row is %d\n", row);
      float temp = new_rank[row];
      int first = in_indices[row];
      int last = in_indices[row+1];
//      bsg_printf("Handling row %d, dcsr d is %d, first is %d, last is %d, bsg_id is %d\n", row, d, first, last, __bsg_id);
      int s = first;
      for(; s <= last - 4; s = s + 4) {
        register int idx0 = in_neighbors[s];
        register int idx1 = in_neighbors[s+1];
        register int idx2 = in_neighbors[s+2];
        register int idx3 = in_neighbors[s+3];
      
        register float tmp_0 = contrib[idx0];
        register float tmp_1 = contrib[idx1];
        register float tmp_2 = contrib[idx2];
        register float tmp_3 = contrib[idx3];
       
        temp = temp + tmp_0 + tmp_1 + tmp_2 + tmp_3; 
      } //end of loop on in neighbors
      for(; s < last; s++) {
        register int idx = in_neighbors[s];
        register float tmp = contrib[idx];
        temp = temp + tmp;
      }
      new_rank[row] = temp;
    }
  }
  bsg_barrier_hw_tile_group_sync();
  bsg_cuda_print_stat_end(tag);
  return 0;
}

static inline void align_range(int V, int* start, int * end) {
  *start = CACHE_LINE * bsg_id;
  int temp_end = *start + CACHE_LINE;
  *end = (temp_end > V) ? V : temp_end;  
}

extern "C" int  __attribute__ ((noinline)) computeContrib_kernel(float * contrib, float * old_rank, int * out_degree, int V) {
  bsg_cuda_print_stat_start(1);
  bsg_barrier_hw_tile_group_init();
  int start, end;
//  int rows_within_pod = (V % KERNEL_NUM_PODS) == 0 ? (V / KERNEL_NUM_PODS) : (V / KERNEL_NUM_PODS + 1);
  align_range(V, &start, &end);
  register float one = ((float) 1);
  int cache_total = CACHE_LINE * bsg_tiles_X * bsg_tiles_Y;
  pr_dbg("compute contrib start: %i, end: %i, id: %i, and total number of nodes %d\n", start, end, bsg_id, V);
  for (; start < V; start = start + cache_total) {
    end = start + CACHE_LINE;
    end = end > V ? V : end;
    int d = start;
    for(; d <= end - 8; d = d + 8) {
      register int od0 = out_degree[d];
      register int od1 = out_degree[d+1];
      register int od2 = out_degree[d+2];
      register int od3 = out_degree[d+3];
      register int od4 = out_degree[d+4];
      register int od5 = out_degree[d+5];
      register int od6 = out_degree[d+6];
      register int od7 = out_degree[d+7];

      register float tp_old0 = old_rank[d];
      register float tp_old1 = old_rank[d+1];
      register float tp_old2 = old_rank[d+2];
      register float tp_old3 = old_rank[d+3];
      register float tp_old4 = old_rank[d+4];
      register float tp_old5 = old_rank[d+5];
      register float tp_old6 = old_rank[d+6];
      register float tp_old7 = old_rank[d+7];     
     
      register float cont0 = tp_old0 / od0;
      register float cont1 = tp_old1 / od1;
      register float cont2 = tp_old2 / od2;
      register float cont3 = tp_old3 / od3;
      register float cont4 = tp_old4 / od4;
      register float cont5 = tp_old5 / od5;
      register float cont6 = tp_old6 / od6;
      register float cont7 = tp_old7 / od7;

      contrib[d] = cont0;
      contrib[d+1] = cont1;
      contrib[d+2] = cont2;
      contrib[d+3] = cont3;
      contrib[d+4] = cont4;
      contrib[d+5] = cont5;
      contrib[d+6] = cont6;
      contrib[d+7] = cont7;
    }
    for(; d < end; d++) {
      register int od = out_degree[d];
      register float tp_old = old_rank[d];
      register float cont = tp_old / od;
      contrib[d] = cont;
      
    }
  }
  bsg_cuda_print_stat_end(1);
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();
  return 0;
}
extern "C" int __attribute__ ((noinline)) edgeset_apply_pull_serial_call(int *out_indices, int *out_neighbors, float * new_rank, float * contrib, int V) {
  edgeset_apply_pull_serial(out_indices, out_neighbors, new_rank, contrib, V);
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();
  return 0;
}
extern "C" int  __attribute__ ((noinline)) updateVertex_kernel(float * old_rank, float *new_rank, float * error, int V, int E) {
  bsg_cuda_print_stat_start(3);
  bsg_barrier_hw_tile_group_init();
  register float zero = ((float) 0);
  int rows_within_pod = (V - KERNEL_CURRENT_POD) % KERNEL_NUM_PODS == 0 ?
                        ((V - KERNEL_CURRENT_POD) / KERNEL_NUM_PODS) : ((V - KERNEL_CURRENT_POD) / KERNEL_NUM_PODS + 1);
  int thread_num = bsg_tiles_X * bsg_tiles_Y;
  int length_each_tile = rows_within_pod % thread_num == 0 ? (rows_within_pod / thread_num) : (rows_within_pod / thread_num + 1);

  int start = KERNEL_CURRENT_POD + __bsg_id * KERNEL_NUM_PODS * length_each_tile;
  int end = start + KERNEL_NUM_PODS * length_each_tile;
  end = end < V ? end : V;
  start = start < V ? start : V;
  int d = start;
  for(; d < end - 7 * KERNEL_NUM_PODS; d = d + 8 * KERNEL_NUM_PODS) {
//    bsg_printf("d inside unroll is %d, bsg_id is %d\n", d, __bsg_id);
    register float tp_new0 = new_rank[d];
    register float tp_new1 = new_rank[d+KERNEL_NUM_PODS];
    register float tp_new2 = new_rank[d+2 * KERNEL_NUM_PODS];
    register float tp_new3 = new_rank[d+3 * KERNEL_NUM_PODS];
    register float tp_new4 = new_rank[d+4 * KERNEL_NUM_PODS];
    register float tp_new5 = new_rank[d+5 * KERNEL_NUM_PODS];
    register float tp_new6 = new_rank[d+6 * KERNEL_NUM_PODS];
    register float tp_new7 = new_rank[d+7 * KERNEL_NUM_PODS];

    register float tp_old0 = old_rank[d];
    register float tp_old1 = old_rank[d+KERNEL_NUM_PODS];
    register float tp_old2 = old_rank[d+2*KERNEL_NUM_PODS];
    register float tp_old3 = old_rank[d+3* KERNEL_NUM_PODS];
    register float tp_old4 = old_rank[d+4* KERNEL_NUM_PODS];
    register float tp_old5 = old_rank[d+5* KERNEL_NUM_PODS];
    register float tp_old6 = old_rank[d+6* KERNEL_NUM_PODS];
    register float tp_old7 = old_rank[d+7* KERNEL_NUM_PODS];

    tp_new0 = beta_score + damp * tp_new0;
    tp_new1 = beta_score + damp * tp_new1;
    tp_new2 = beta_score + damp * tp_new2;
    tp_new3 = beta_score + damp * tp_new3;
    tp_new4 = beta_score + damp * tp_new4;
    tp_new5 = beta_score + damp * tp_new5;
    tp_new6 = beta_score + damp * tp_new6;
    tp_new7 = beta_score + damp * tp_new7;

    error[d] = fabs(tp_new0 - tp_old0);
    error[d+KERNEL_NUM_PODS] = fabs(tp_new1 - tp_old1);
    error[d+2*KERNEL_NUM_PODS] = fabs(tp_new2 - tp_old2);
    error[d+3*KERNEL_NUM_PODS] = fabs(tp_new3 - tp_old3);
    error[d+4*KERNEL_NUM_PODS] = fabs(tp_new4 - tp_old4);
    error[d+5*KERNEL_NUM_PODS] = fabs(tp_new5 - tp_old5);
    error[d+6*KERNEL_NUM_PODS] = fabs(tp_new6 - tp_old6);
    error[d+7*KERNEL_NUM_PODS] = fabs(tp_new7 - tp_old7);

    old_rank[d] = tp_new0;
    old_rank[d+KERNEL_NUM_PODS] = tp_new1;
    old_rank[d+2*KERNEL_NUM_PODS] = tp_new2;
    old_rank[d+3*KERNEL_NUM_PODS] = tp_new3;
    old_rank[d+4*KERNEL_NUM_PODS] = tp_new4;
    old_rank[d+5*KERNEL_NUM_PODS] = tp_new5;
    old_rank[d+6*KERNEL_NUM_PODS] = tp_new6;
    old_rank[d+7*KERNEL_NUM_PODS] = tp_new7;

    new_rank[d] = zero;
    new_rank[d+KERNEL_NUM_PODS] = zero;
    new_rank[d+2*KERNEL_NUM_PODS] = zero;
    new_rank[d+3*KERNEL_NUM_PODS] = zero;
    new_rank[d+4*KERNEL_NUM_PODS] = zero;
    new_rank[d+5*KERNEL_NUM_PODS] = zero;
    new_rank[d+6*KERNEL_NUM_PODS] = zero;
    new_rank[d+7*KERNEL_NUM_PODS] = zero;   
  }
  for(; d < end; d = d + KERNEL_NUM_PODS) {
//    bsg_printf("d is %d, bsg_id is %d\n", d, __bsg_id);
    register float tp_new = new_rank[d];
    register float tp_old = old_rank[d];
    tp_new = beta_score + damp * tp_new;
    error[d] = fabs(tp_new - tp_old);
    old_rank[d] = tp_new;
    new_rank[d] = zero;
  }
  bsg_cuda_print_stat_end(3);
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();
  return 0;
}