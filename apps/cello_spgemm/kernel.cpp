#include <cello/cello.hpp>
#include <util/statics.hpp>
#include <algorithm>
#include "common.hpp"

DRAM(csx_type) A;
DRAM(csx_type) B;
DRAM(csx_type) C;
DRAM(list_head_vector) C_list_head;
DRAM(count_vector) C_col_count;

#define fmadd_asm(rd_p, rs1_p, rs2_p, rs3_p)                            \
    asm volatile ("fmadd.s %[rd], %[rs1], %[rs2], %[rs3]"               \
                  : [rd] "=f" (rd_p)                                    \
                  : [rs1] "f" ((rs1_p)), [rs2] "f" ((rs2_p)), [rs3] "f" ((rs3_p)))


#define NUM_DMEM_NODES 1
#define LIST_NULL_PTR ((HBListNode*) 0)

// Free nodes;
DMEM(HBList)     free_dram_nodes;
DMEM(HBList)     free_dmem_nodes;
DMEM(HBListNode) dmem_nodes [NUM_DMEM_NODES];

static void        list_init(HBList* list);
static HBListNode* list_pop_front(HBList* list);
static bool        list_empty(HBList* list);
static HBListNode* get_new_dram_node();
static HBListNode* get_new_dmem_node();
static void        list_append_back(HBList* list, HBListNode* node);
static bool        is_dram_node(HBListNode* node);
static void        list_free(HBListNode* node);
static void        list_free_dmem_node(HBListNode* node);
static void        list_concat(HBList* list1, HBList* list2);

/**
 * @brief initialize the free lists
 */
static void init_spgemm()
{
    // initialize the free list;
    if (cello::my::tile_id() == 0) {
        
    }

    // initialize the free list;
    list_init(&free_dram_nodes);

    for (int i = 0; i < NUM_DMEM_NODES; i++) {
        HBListNode *new_node = &dmem_nodes[i];
        new_node->next = (HBListNode*) 0;
        list_append_back(&free_dmem_nodes, new_node);
    }

    bsg_fence();
}
cello_constructor(init_spgemm);

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
static inline HBListNode* get_new_dram_node() {
    // free local nodes ran out; fall back to DRAM;
    if (list_empty(&free_dram_nodes)) {
        // Nothing in hand; Get more from the common pool;
        HBListNode*new_node = static_cast<HBListNode*>(cello::allocate(sizeof(HBListNode)));
        //new_node->next = (HBListNode*) 0; probably don't need this;
        return new_node;
    } else {
        // Get it from the private free list;
        return list_pop_front(&free_dram_nodes);
    }
}

// Try getting new dmem node; fall back to DRAM;
static inline HBListNode* get_new_dmem_node() {
    if (list_empty(&free_dmem_nodes)) {
        return get_new_dram_node();
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

int cello_main(int argc, char *argv[])
{
    bsg_printf("A: %d rows, %d columns, %d non-zeros\n", A.rows(), A.cols(), A.nnz());
    bsg_printf("B: %d rows, %d columns, %d non-zeros\n", B.rows(), B.cols(), B.nnz());
    C_list_head.foreach([](int i, HBListNodePtr &row_result) {
        auto [A_idx_start, A_idx_end, A_val_start, A_val_end] = A.inner_indices_values_range_lcl(i);        
        asm volatile ("" ::: "memory");

        HBList accum_row;
        list_init(&accum_row);
        
        value_type *A_col_val = A_val_start;
        for (index_type *A_col_idx_p = A_idx_start;
             A_col_idx_p != A_idx_end;
             A_col_idx_p++, A_col_val++) {
            index_type A_col_idx = *A_col_idx_p;
            value_type A_val = *A_col_val;
            asm volatile ("" ::: "memory");

            auto [B_idx_start, B_idx_end, B_val_start, B_val_end] = B.inner_indices_values_range(A_col_idx);
            asm volatile ("" ::: "memory");

            HBList curr_row;
            auto B_col_val = B_val_start;
            for (auto B_col_idx_p = B_idx_start;
                 B_col_idx_p != B_idx_end;
                 B_col_idx_p++, B_col_val++) {
                index_type B_col_idx = *B_col_idx_p;
                value_type B_val = *B_col_val;
                asm volatile ("" ::: "memory");

                HBListNode* new_node = get_new_dmem_node();
                new_node->col_idx = B_col_idx;
                new_node->nnz = A_val * B_val;
                new_node->next = (HBListNode*) 0;

                list_append_back(&curr_row, new_node);
            }

            // merge row
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
                            fmadd_asm(accum_node->nnz, A_val, curr_node->nnz, accum_node->nnz);
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

            // convert all the dmem nodes into dram nodes;
            HBListNode* prev_node = (HBListNode*) 0;
            HBListNode* curr_node = accum_row.head;

            while (curr_node != (HBListNode*) 0) {
                HBListNode* next_node = curr_node->next;

                if (!is_dram_node(curr_node)) {
                    // make a new DRAM node;
                    HBListNode* new_node = get_new_dram_node();

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

            // store back
            C_list_head.local(i) = accum_row.head;
            C_col_count.local(i) = accum_row.count;
        }
    });
    return 0;
}
