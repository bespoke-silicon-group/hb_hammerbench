#pragma once
#include <algorithm>
#include "sparse_matrix.h"
#include "spmm.hpp"
#include "spmm_compute_offsets_common.hpp"
#include "bsg_tile_config_vars.h"
#include "bsg_mcs_mutex.hpp"
#include "bsg_manycore_arch.h"
#include "bsg_manycore.h"
#include "spmm_barrier.hpp"

__attribute__((section(".dram")))
static std::atomic<int> nnz_sum_tree[2*THREADS];

#define LEVELS                                  \
    (tree_levels(THREADS))
__attribute__ ((always_inline))
static int ceil_log2(int x)
{
    int j = 0;
    while (x > (1<<j)) {
        j = j+1;
    }
    return j;
}
__attribute__ ((always_inline))
static int tree_levels(int leafs)
{
    return ceil_log2(leafs)+1;
}
__attribute__ ((always_inline))
static int sum_tree_rchild(int root)
{
    return 2*root + 2;
}
__attribute__ ((always_inline))
static int sum_tree_lchild(int root)
{
    return 2*root + 1;
}

__attribute__ ((always_inline))
static void sum_tree_update(int sum)
{
    int r = 0;
    int m = THREADS;
    int L = LEVELS;
    bsg_unroll(1)
    for (int l = 0; l < L; l++) {
        // pr_dbg("%d: updating tree[%d] += %d\n"
        //             , __bsg_id
        //             , r
        //             , sum);
        nnz_sum_tree[r].fetch_add(sum, std::memory_order_relaxed);
        m >>= 1;
        if (m & __bsg_id) {
            r = sum_tree_rchild(r);
        } else {
            r = sum_tree_lchild(r);
        }
    }
    // pr_dbg("%d: updating tree[%d] += %d\n"
    //             , __bsg_id
    //             , r
    //             , sum);
    //nnz_sum_tree[r].fetch_add(sum, std::memory_order_relaxed);
}

static int sum_tree_accumulate()
{
    int s = 0;
    int r = 0;
    int m = THREADS;
    int L = LEVELS;
    bsg_unroll(1)
    for (int l = 0; l < L; l++) {
        m >>= 1;
        // pr_dbg("%d: __bsg_id & 0x%08x = %d\n"
        //            , __bsg_id
        //            , m
        //            , __bsg_id & m);
        if (__bsg_id & m) {
            s += nnz_sum_tree[sum_tree_lchild(r)].load(std::memory_order_relaxed);
            // pr_dbg("%d: accumulating tree[%d], sum = %d\n"
            //             , __bsg_id
            //             , sum_tree_lchild(r)
            //             , s);
            r = sum_tree_rchild(r);
        } else {
            r = sum_tree_lchild(r);
        }
    }
    return s;
}


void spmm_compute_offsets()
{
    // 1. compute offsets of your work region
#ifdef __PART__
    int mjr_range = C_part_lcl.partinfo.major_stop+1 - C_part_lcl.partinfo.major_start;
    int region_size = (mjr_range + THREADS - 1)/THREADS;
    int start = C_part_lcl.partinfo.major_start + __bsg_id * region_size;
    int end = std::min(start + region_size, C_part_lcl.partinfo.major_stop+1);
#else
    int region_size = (C_lcl.n_major + THREADS)/THREADS;
    int start = __bsg_id * region_size;
    int end   = std::min(start + region_size, C_lcl.n_major+1);
#endif
    if (start < end) {
        pr_dbg("%s: start = %3d, end = %3d\n"
               , __func__
               , start
               , end);
    }
    // zero-out the sum tree
    bsg_unroll(1)
    for (int i = __bsg_id; i < ARRAY_SIZE(nnz_sum_tree); i += bsg_tiles_X*bsg_tiles_Y) {
        nnz_sum_tree[i].store(0, std::memory_order_relaxed);
    }
    //barrier::spmm_barrier();
    bsg_fence();
    bsg_barrier_hw_tile_group_sync();


    // pr_dbg("%s: %d: start = %d, end = %d\n"
    //             , __func__
    //             , __bsg_id
    //             , start
    //             , end);

    int sum = 0;
    bsg_unroll(1)
    for (int Ci = start; Ci < end; Ci++) {
        // update offsets
        pr_dbg("%s: preoff[%3d]=%3d\n"
               , __func__
               , Ci
               , sum);
        C_lcl.mnr_off_remote_ptr[Ci] = sum;
        sum += C_lcl.mjr_nnz_remote_ptr[Ci];
    }

    // 2. update the sum tree
    sum_tree_update(sum);
    //barrier::spmm_barrier();
    bsg_fence();
    bsg_barrier_hw_tile_group_sync();

    // 3. propogate updates from the sum tree to your work region
    int offset = sum_tree_accumulate();
    bsg_unroll(1)
    for (int Ci = start; Ci < end; Ci++) {
        std::atomic<int>* nnzp = reinterpret_cast<std::atomic<int>*>(&C_lcl.mnr_off_ptr[Ci]);
        nnzp->fetch_add(offset, std::memory_order_relaxed);
        pr_dbg("%s: offset[%3d]=%3d\n"
               , __func__
               , Ci
               , nnzp->load(std::memory_order_relaxed));
    }
}
