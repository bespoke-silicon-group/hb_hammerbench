#include <bsg_cuda_lite_barrier.h>
#include <bsg_manycore_atomic.h>
#include <bsg_manycore.h>
#include <cstdint>
#include <algorithm>
#include "hb_list.h"
#include "tg_amoadd_barrier.h"

#define fmadd_asm(rd_p, rs1_p, rs2_p, rs3_p) \
    asm volatile ("fmadd.s %[rd], %[rs1], %[rs2], %[rs3]" \
      : [rd] "=f" (rd_p) \
      : [rs1] "f" ((rs1_p)), [rs2] "f" ((rs2_p)), [rs3] "f" ((rs3_p)))


#define INIT_NUM_DRAM_NODES 256
#define NUM_DMEM_NODES 256
#define NUM_DRAM_NODES 16777216 // (=2**24)
#define LIST_NULL_PTR ((HBListNode*) 0)

// tg amoadd barrier;
__attribute__((section(".dram"))) int tg_lock[NUM_TG];
int sense;

// amoadd queue;
__attribute__((section(".dram"))) int g_free_node_q[NUM_TG];
__attribute__((section(".dram"))) int g_solve_q[NUM_TG];
__attribute__((section(".dram"))) int g_convert_q[NUM_TG];


// Sum tree;
#define NUM_TILES (bsg_tiles_X*bsg_tiles_Y)
#define NUM_TILES_PER_TG (bsg_tiles_X*bsg_tiles_Y/NUM_TG) 
#define SUM_TREE_LEN ((2*NUM_TILES_PER_TG)-1)
 __attribute__((section(".dram"))) int sum_tree[SUM_TREE_LEN*NUM_TG];

// TG ID;
int tgid;
int tid;
int *tg_free_node_q;
int *tg_solve_q;
int *tg_convert_q;

int *tg_B_row_offset;
int *tg_B_col_idx;
float *tg_B_nnz;
int *tg_C_row_offset;
int *tg_C_col_idx;
float *tg_C_nnz;

int *tg_C_col_count;
HBListNode** tg_C_list_head;
HBListNode* tg_dram_nodes;
int *tg_sum_tree;

// Free nodes;
HBList free_dram_nodes;
HBList free_dmem_nodes;
HBListNode dmem_nodes[NUM_DMEM_NODES];

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
static inline HBListNode* get_new_dram_node(int* q, HBListNode* dram_nodes) {
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

// Try getting new dmem node; fall back to DRAM;
static inline HBListNode* get_new_dmem_node(int* q, HBListNode* dram_nodes) {
  if (list_empty(&free_dmem_nodes)) {
    return get_new_dram_node(q, dram_nodes);
  } else {
    return list_pop_front(&free_dmem_nodes);
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

static inline bool is_dram_node(HBListNode* node) {
  intptr_t ptr = reinterpret_cast<intptr_t>(node);
  return ptr & 0x80000000;
}

// Return the node to the free list;
static inline void list_free(HBListNode* node) {
  if (is_dram_node(node)) {
    list_append_back(&free_dram_nodes, node);
  } else {
    list_append_back(&free_dmem_nodes, node);
  }
}

static inline void list_free_dmem_node(HBListNode* node) {
  list_append_back(&free_dmem_nodes, node);
}

static inline void list_concat(HBList* list1, HBList* list2) {
  if (list_empty(list2)) {
    return;
  } else {
    if (list_empty(list1)) {
      list1->count = list2->count;
      list1->head = list2->head;
      list1->tail = list2->tail;
    } else {
      list1->count += list2->count;
      list1->tail->next = list2->head;
      list1->tail = list2->tail;
    }
  }
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
  // num rows;
  int num_row,
  int B_row_offset_size,
  int B_col_idx_size,
  int C_row_offset_size,
  int C_col_idx_size
)
{
  bsg_barrier_hw_tile_group_init();
  bsg_barrier_hw_tile_group_sync();

  // TG Id;
  tgid = __bsg_id / NUM_TILES_PER_TG;
  tid  = __bsg_id % NUM_TILES_PER_TG;
  tg_free_node_q = &g_free_node_q[tgid];
  tg_solve_q = &g_solve_q[tgid];
  tg_convert_q = &g_convert_q[tgid];

  tg_B_row_offset = &B_row_offset[tgid*B_row_offset_size];
  tg_B_col_idx = &B_col_idx[tgid*B_col_idx_size];
  tg_B_nnz = &B_nnz[tgid*B_col_idx_size];
  tg_C_row_offset = &C_row_offset[tgid*C_row_offset_size];
  tg_C_col_idx = &C_col_idx[tgid*C_col_idx_size];
  tg_C_nnz = &C_nnz[tgid*C_col_idx_size];

  tg_C_col_count = &C_col_count[tgid*(num_row+1)];
  tg_C_list_head = &C_list_head[tgid*num_row];
  tg_dram_nodes = &dram_nodes[tgid*(NUM_DRAM_NODES/NUM_TG)];
  tg_sum_tree = &sum_tree[tgid*SUM_TREE_LEN];

  // Initialize amo variables + sum tree;
  if (tid == 0) {
    *tg_free_node_q = 0;
    *tg_solve_q = 0;
    *tg_convert_q = 0;
    for (int i = 0; i < SUM_TREE_LEN; i++) {
      tg_sum_tree[i] = 0;
    }
    tg_lock[tgid] = 0;
  }
  sense=1;
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();

  // Initialize dram free nodes;
  list_init(&free_dram_nodes);
  int start_idx = bsg_amoadd(tg_free_node_q, INIT_NUM_DRAM_NODES);
  for (int i = 0; i < INIT_NUM_DRAM_NODES; i++) {
    HBListNode* new_node = &tg_dram_nodes[start_idx+i];
    new_node->next = (HBListNode*) 0;
    list_append_back(&free_dram_nodes, new_node);
  }
  bsg_fence();

  // Initialize free dmem nodes;
  list_init(&free_dmem_nodes);
  for (int i = 0; i < NUM_DMEM_NODES; i++) {
    HBListNode* new_node = &dmem_nodes[i];
    new_node->next = (HBListNode*) 0;
    list_append_back(&free_dmem_nodes, new_node);
  }

  //  barrier;
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();
  bsg_cuda_print_stat_kernel_start();

  // KERNEL START;
  // solve rows;
  for (int curr_row = bsg_amoadd(tg_solve_q,1); curr_row < num_row; curr_row=bsg_amoadd(tg_solve_q,1)) {
    //bsg_print_int(curr_row);
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
      int B_col_idx_start = tg_B_row_offset[A_col_idx0];
      int B_col_idx_end   = tg_B_row_offset[A_col_idx0+1];
      asm volatile ("" ::: "memory");
    
      HBList curr_row;
      list_init(&curr_row);

      // unroll by 1;
      int b = B_col_idx_start;
      for (; b < B_col_idx_end; b+=1) {
        int B_col_idx0 = tg_B_col_idx[b];
        float B_nnz0 = tg_B_nnz[b];
        asm volatile ("" ::: "memory");
        // make new node;
        HBListNode* new_node = get_new_dmem_node(tg_free_node_q, tg_dram_nodes);
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
            list_concat(&temp_row, &curr_row);
            break;
          }
        } else {
          if (is_curr_empty) {
            // pop accum;
            list_concat(&temp_row, &accum_row);
            break;
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
  
    // convert all the dmem nodes into dram nodes;
    HBListNode* prev_node = (HBListNode*) 0;
    HBListNode* curr_node = accum_row.head;

    while (curr_node != (HBListNode*) 0) {
      HBListNode* next_node = curr_node->next;

      if (!is_dram_node(curr_node)) {
        // make a new DRAM node;
        HBListNode* new_node = get_new_dram_node(tg_free_node_q, tg_dram_nodes);

        // update the pointers on the list object;
        if (curr_node == accum_row.head) {
          accum_row.head = new_node;
        }
        if (curr_node == accum_row.tail) {
          accum_row.tail = new_node;
        }

        // copy data to the new node;
        new_node->col_idx = curr_node->col_idx;
        new_node->nnz = curr_node->nnz;
        // connect to the next;
        new_node->next = next_node;
        // connect to the prev;
        if (prev_node != (HBListNode*) 0) {
          prev_node->next = new_node;
        }
        // free dmem
        list_free_dmem_node(curr_node);
        // next prev node is new node;;
        prev_node = new_node;
      } else {
        prev_node = curr_node;
      }

      curr_node = next_node;
    }
  

    // set it to C_list_head;
    tg_C_list_head[curr_row] = accum_row.head;
    tg_C_col_count[curr_row] = accum_row.count;
  
    //bsg_print_int(10000+curr_row);
  }
  bsg_fence();
  tg_amoadd_barrier(&tg_lock[tgid], &sense, tgid*(8/NUM_TG), (8/NUM_TG));
  if(tid == 0) {
    //bsg_print_int(20000);
  }

  // calculate C row offset;
  int row_per_tile = (num_row+NUM_TILES_PER_TG)/NUM_TILES_PER_TG;
  int r_start = tid*row_per_tile;
  int r_end = std::min(r_start+row_per_tile, num_row+1);
  
  // calculate sum;
  int sum = 0;
  for (int i = r_start; i < r_end; i++) {
    tg_C_row_offset[i] = sum;
    sum += tg_C_col_count[i];
  }

  // update sum tree;
  int r = 0;
  int m = NUM_TILES_PER_TG;
  for (int l = 0; l < TREE_LEVELS; l++) {
    bsg_amoadd(&tg_sum_tree[r], sum);
    m >>= 1;
    if (m & tid) {
      r = (2*r)+2;
    } else {
      r = (2*r)+1;
    }
  }
  bsg_fence();
  tg_amoadd_barrier(&tg_lock[tgid], &sense, tgid*(8/NUM_TG), (8/NUM_TG));

  // collect from sum  tree;
  int offset = 0;
  r = 0;
  m = NUM_TILES_PER_TG;
  for (int l = 0; l < TREE_LEVELS; l++) {
    m >>= 1;
    if (tid & m) {
      offset += tg_sum_tree[(2*r)+1];
      r = (2*r)+2;
    } else {
      r = (2*r)+1;
    }
  }

  // update with offset;
  for (int i = r_start; i < r_end; i++) {
    bsg_amoadd(&tg_C_row_offset[i], offset);
  }

  bsg_fence();
  tg_amoadd_barrier(&tg_lock[tgid], &sense, tgid*(8/NUM_TG), (8/NUM_TG));

  if(tid == 0) {
    //bsg_print_int(30000);
  }

  // Convert lists into CSR matrix;
  for (int curr_row = bsg_amoadd(tg_convert_q,1); curr_row < num_row; curr_row=bsg_amoadd(tg_convert_q,1)) {
    //bsg_print_int(40000+curr_row);
    HBListNode* curr_node = tg_C_list_head[curr_row];
    int row_offset = tg_C_row_offset[curr_row];
    while (curr_node != (HBListNode*) 0) {
      tg_C_col_idx[row_offset] = curr_node->col_idx;
      tg_C_nnz[row_offset] = curr_node->nnz;
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