#include <bsg_cuda_lite_barrier.h>
#include "bsg_barrier_multipod.h"
#include <bsg_manycore_atomic.h>
#include <bsg_manycore.h>
#include "hb_list.h"

#define fmadd_asm(rd_p, rs1_p, rs2_p, rs3_p) \
    asm volatile ("fmadd.s %[rd], %[rs1], %[rs2], %[rs3]" \
      : [rd] "=f" (rd_p) \
      : [rs1] "f" ((rs1_p)), [rs2] "f" ((rs2_p)), [rs3] "f" ((rs3_p)))


#define INIT_NUM_DRAM_NODES 2048
#define LIST_NULL_PTR ((HBListNode*) 0)

// multipod barrier;
volatile int done[NUM_POD_X]={0};
int alert = 0;

// amoadd queue;
__attribute__((section(".dram"))) int g_free_node_q;
__attribute__((section(".dram"))) int g_solve_q;
__attribute__((section(".dram"))) int g_convert_q;


// Free nodes;
HBList free_dram_nodes;


// List init;
static inline void list_init(HBList* list) {
  list->count = 0;
  list->head = (HBListNode*) 0;
  list->tail = (HBListNode*) 0;
}

// Assumes that list is not empty;
static inline HBListNode* list_pop_front(HBList* list) {
  HBListNode* ret = list->head;
  HBListNode* new_head = ret->next;
  list->head = new_head;

  // it was the last node;
  if (new_head == (HBListNode*) 0) {
    list->tail = (HBListNode*) 0;
  }

  ret->next = (HBListNode*) 0;
  list->count--;
  return ret;
}

// Is the list empty?
static inline bool list_empty(HBList* list) {
  return (list->count ==  0);
}


// Get a new node;
static inline HBListNode* get_new_node(int* q, HBListNode* dram_nodes) {
  // free local nodes ran out; fall back to DRAM;
  if (list_empty(&free_dram_nodes)) {
    // Nothing in hand; Get more from the common pool;
    int id = bsg_amoadd(q,1);
    return &dram_nodes[id];
  } else {
    // Get it from the private free list;
    return list_pop_front(&free_dram_nodes);
  }
}


// Append back to the list;
static inline void list_append_back(HBList* list, HBListNode* node) {
  if (list_empty(list)) {
    // list is empty;
    list->head = node;
    list->tail = node;
  } else {
    list->tail->next = node; 
    list->tail = node;
  }
  list->count++;
}


// Return the node to the free list;
static inline void list_free(HBListNode* node) {
  list_append_back(&free_dram_nodes, node);
}




// Kernel main;
extern "C" int kernel(
  // matrix A;
  int *A_row_offset,
  int *A_col_idx,
  float *A_nnz,
  // matrix B;
  int *B_row_offset,
  int *B_col_idx,
  float *B_nnz,
  // matrix C;
  int *C_row_offset,
  int *C_col_idx,
  float *C_nnz,
  // needed by algorithm
  int *C_col_count,
  HBListNode** C_list_head,
  HBListNode* dram_nodes,
  // id;
  int num_row,
  int pod_id
)
{
  bsg_barrier_hw_tile_group_init();
  bsg_barrier_hw_tile_group_sync();
  // Initialize amo variables;
  if (__bsg_id == 0) {
    g_free_node_q = 0;
    g_solve_q = 0;
    g_convert_q = 0;
  }
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();


  // Initialize dram free nodes;
  list_init(&free_dram_nodes);
  int start_idx = bsg_amoadd(&g_free_node_q, INIT_NUM_DRAM_NODES);
  for (int i = 0; i < INIT_NUM_DRAM_NODES; i++) {
    HBListNode* new_node = &dram_nodes[start_idx+i];
    new_node->next = (HBListNode*) 0;
    list_append_back(&free_dram_nodes, new_node);
  }
  bsg_fence();


  // Multi-pod barrier;
  bsg_barrier_multipod(pod_id, NUM_POD_X, done, &alert);
  bsg_cuda_print_stat_kernel_start();

  // KERNEL START;
  // solve rows;
  for (int curr_row = bsg_amoadd(&g_solve_q,1); curr_row < num_row; curr_row=bsg_amoadd(&g_solve_q,1)) {
    int A_col_idx_start = A_row_offset[curr_row];
    int A_col_idx_end = A_row_offset[curr_row+1];
    asm volatile ("" ::: "memory");

    HBList accum_row;
    list_init(&accum_row);

    bsg_unroll(1)
    for (int s = A_col_idx_start; s < A_col_idx_end; s++) {
      int A_col_idx0 = A_col_idx[s];
      float A_nnz0 = A_nnz[s];
      asm volatile ("" ::: "memory");
      int B_col_idx_start = B_row_offset[A_col_idx0];
      int B_col_idx_end   = B_row_offset[A_col_idx0+1];
      asm volatile ("" ::: "memory");
    
      HBList curr_row;
      list_init(&curr_row);

      // unroll by 1;
      int b = B_col_idx_start;
      for (; b < B_col_idx_end; b+=1) {
        int B_col_idx0 = B_col_idx[b];
        float B_nnz0 = B_nnz[b];
        asm volatile ("" ::: "memory");
        // make new node;
        HBListNode* new_node = get_new_node(&g_free_node_q, dram_nodes);
        new_node->col_idx = B_col_idx0;
        new_node->nnz = B_nnz0;
        new_node->next = (HBListNode*) 0;
        list_append_back(&curr_row, new_node);
      }

      // merge row;
      HBList temp_row;
      list_init(&temp_row);
      
      while (true) {
        bool is_accum_empty = list_empty(&accum_row);
        bool is_curr_empty = list_empty(&curr_row);
        if (is_accum_empty) {
          if (is_curr_empty) {
            // both lists empty;
            break;
          } else {
            // pop curr;
            HBListNode* node = list_pop_front(&curr_row);
            list_append_back(&temp_row, node);
          }
        } else {
          if (is_curr_empty) {
            // pop accum;
            HBListNode* node = list_pop_front(&accum_row);
            list_append_back(&temp_row, node);
          } else {
            // compare two fronts;
            int accum_front = accum_row.head->col_idx;
            int curr_front = curr_row.head->col_idx;
            if (accum_front == curr_front) {
              // merge two nodes;
              HBListNode* curr_node  = list_pop_front(&curr_row);
              HBListNode* accum_node = list_pop_front(&accum_row);
              fmadd_asm(accum_node->nnz, A_nnz0, curr_node->nnz, accum_node->nnz);
              list_append_back(&temp_row, accum_node);
              list_free(curr_node);
            } else if (accum_front > curr_front) {
              // pop curr;
              HBListNode *curr_node = list_pop_front(&curr_row);
              list_append_back(&temp_row, curr_node);
            } else {
              // pop accum;
              HBListNode *accum_node = list_pop_front(&accum_row);
              list_append_back(&temp_row, accum_node);
            }
          }
        }
      }

      // Switch;
      accum_row.head = temp_row.head;
      accum_row.tail = temp_row.tail;
      accum_row.count = temp_row.count;
    }

    // set it to C_list_head;
    C_list_head[curr_row] = accum_row.head;
    C_col_count[curr_row] = accum_row.count;
  }
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();

  // calculate C row offset;
  if (__bsg_id == 0) {
    int r = 0;
    int accum=0;
    // unroll by 16;
    for (; r + 15 < num_row; r+=16) {
      int c0 = C_col_count[r+0];
      int c1 = C_col_count[r+1];
      int c2 = C_col_count[r+2];
      int c3 = C_col_count[r+3];
      int c4 = C_col_count[r+4];
      int c5 = C_col_count[r+5];
      int c6 = C_col_count[r+6];
      int c7 = C_col_count[r+7];
      int c8 = C_col_count[r+8];
      int c9 = C_col_count[r+9];
      int c10 = C_col_count[r+10];
      int c11 = C_col_count[r+11];
      int c12 = C_col_count[r+12];
      int c13 = C_col_count[r+13];
      int c14 = C_col_count[r+14];
      int c15 = C_col_count[r+15];
      asm volatile ("" ::: "memory");
      C_row_offset[r+0] = accum;
      accum += c0;
      C_row_offset[r+1] = accum;
      accum += c1;
      C_row_offset[r+2] = accum;
      accum += c2;
      C_row_offset[r+3] = accum;
      accum += c3;
      C_row_offset[r+4] = accum;
      accum += c4;
      C_row_offset[r+5] = accum;
      accum += c5;
      C_row_offset[r+6] = accum;
      accum += c6;
      C_row_offset[r+7] = accum;
      accum += c7;
      C_row_offset[r+8] = accum;
      accum += c8;
      C_row_offset[r+9] = accum;
      accum += c9;
      C_row_offset[r+10] = accum;
      accum += c10;
      C_row_offset[r+11] = accum;
      accum += c11;
      C_row_offset[r+12] = accum;
      accum += c12;
      C_row_offset[r+13] = accum;
      accum += c13;
      C_row_offset[r+14] = accum;
      accum += c14;
      C_row_offset[r+15] = accum;
      accum += c15;
      asm volatile ("" ::: "memory");
    }
    // no unroll;
    for (; r < num_row; r++) {
      int c0 = C_col_count[r];
      C_row_offset[r] = accum;
      accum += c0;
    }
    // last one;
    C_row_offset[r] = accum;
  }
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();


  // Convert lists into CSR matrix;
  for (int curr_row = bsg_amoadd(&g_convert_q,1); curr_row < num_row; curr_row=bsg_amoadd(&g_convert_q,1)) {
    HBListNode* curr_node = C_list_head[curr_row];
    int row_offset = C_row_offset[curr_row];
    while (curr_node != (HBListNode*) 0) {
      C_col_idx[row_offset] = curr_node->col_idx;
      C_nnz[row_offset] = curr_node->nnz;
      row_offset++;
      curr_node = curr_node->next;
    }
  }
  bsg_fence();

  // KERNEL END;
  bsg_barrier_hw_tile_group_sync();
  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();
  return 0;
}
