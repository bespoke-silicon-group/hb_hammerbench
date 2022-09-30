#include "bsg_manycore.h"
#include "bsg_manycore.hpp"
#include "bsg_manycore_atomic.h"
#include "bsg_cuda_lite_barrier.h"
#include "bsg_mcs_mutex.h"
#include <algorithm>
__attribute__((section(".dram")))
int g_root;

__attribute__((section(".dram")))
int g_V;
int V;
__attribute__((section(".dram")))
int g_E;
int E;

__attribute__((section(".dram")))
const int *g_fwd_offsets;
const int *fwd_offsets;

__attribute__((section(".dram")))
const int *g_fwd_nonzeros;
const int *fwd_nonzeros;

__attribute__((section(".dram")))
const int *g_rev_offsets;
const int *rev_offsets;

__attribute__((section(".dram")))
const int *g_rev_nonzeros;
const int *rev_nonzeros;

__attribute__((section(".dram")))
int *g_distance;
int *distance;

__attribute__((section(".dram")))
int *g_curr_frontier;
__attribute__((section(".dram")))
int g_curr_frontier_size = 0;
__attribute__((section(".dram")))
int g_curr_frontier_dense = 0;

__attribute__((section(".dram")))
int *g_next_frontier;
__attribute__((section(".dram")))
int  g_next_frontier_size = 0;
__attribute__((section(".dram")))
int  g_next_frontier_dense = 1;

__attribute__((section(".dram")))
int g_rev_not_fwd = 0;

__attribute__((section(".dram")))
int *g_dense_to_sparse_set;

__attribute__((section(".dram")))
int *g_sparse_to_dense_set;

__attribute__((section(".dram")))
int g_mu = 0;
__attribute__((section(".dram")))
int g_mf = 0;

__attribute__((section(".dram")))
bsg_mcs_mutex_t printf_lock = 0;

#define serial(block)                           \
    do {                                        \
        bsg_barrier_hw_tile_group_sync();       \
        if (__bsg_id == 0) {                    \
            block;                              \
        }                                       \
        bsg_fence();                            \
        bsg_barrier_hw_tile_group_sync();       \
    } while (0)

#define DEBUG
#ifdef DEBUG
#define pr_int_dbg(i)                           \
    bsg_print_int(i)
#else
#define pr_int_dbg(i)
#endif

#define VDEBUG
#ifdef  VDEBUG
#define pr_dbg(fmt, ...)                                                \
    do {                                                                \
        bsg_mcs_mutex_node_t __node;                                    \
        bsg_mcs_mutex_node_t*__nptr = bsg_tile_group_remote_pointer(__bsg_x, __bsg_y, &__node); \
        bsg_mcs_mutex_acquire(&printf_lock, &__node, __nptr);           \
        bsg_printf(fmt, ##__VA_ARGS__);                                 \
        bsg_mcs_mutex_release(&printf_lock, &__node, __nptr);            \
    } while (0);
#else
#define pr_dbg(fmt, ...)
#endif

//#define VVDEBUG
#ifdef  VVDEBUG
#define pr_vdbg(fmt, ...)                                               \
    do {                                                                \
        bsg_mcs_mutex_node_t __node;                                    \
        bsg_mcs_mutex_node_t*__nptr = bsg_tile_remote_group_pointer(__bsg_x, __bsg_y, &__node); \
        bsg_mcs_mutex_acquire(&printf_lock, &__node, __nptr);           \
        bsg_printf(fmt, ##__VA_ARGS__);                                 \
        bsg_mcs_mutex_release(&printf_lock, &__node, __nptr);            \
    } while (0);
#else
#define pr_vdbg(fmt, ...)
#endif

#define pfor_break                              \
    do {return 1;} while (0)

template <typename BodyT>
static inline void parallel_foreach_static(int START, int STOP, int STEP, BodyT body)
{
    for (int i = START+__bsg_id*STEP; i < STOP; i += STEP*bsg_tiles_X*bsg_tiles_Y) {
        if (body(i)) break;
    }
}

template <typename BodyT>
static inline void parallel_foreach_dynamic(int &START, int STOP, int STEP, BodyT body) {
    for (int i = bsg_amoadd(&START, STEP);
         i < STOP;
         i = bsg_amoadd(&START, STEP)) {
        if (body(i)) break;
    }
}

static inline void zero_dense_set(int *set)
{
    bsg_barrier_hw_tile_group_sync();
    static constexpr int STRIDE = 32*BLOCK_WORDS;
    parallel_foreach_static(0, V, STRIDE, [=](int v_start){
            int v_stop = v_start+STRIDE;
            for (int v = v_start; v < v_stop; v += 32) {
                int idx = v/32;
                set[idx] = 0;
            }
            return 0;
        });
    bsg_barrier_hw_tile_group_sync();
}

static inline int in_dense(int member, int *set)
{
    int idx = member/32;
    int bit = member%32;
    int w = set[idx];
    return w & (1<<bit);
}

static inline void insert_into_dense(int member, int *set, int *set_size)
{
    int idx = member/32;
    int bit = member%32;
    int w = (1<<bit);
    bsg_amoor(&set[idx], w);
    bsg_amoadd(set_size, 1);
}

static inline int *to_sparse(int *set, int set_size, int set_is_dense)
{
    __attribute__((section(".dram"))) static int new_set_size = 0;
    serial({
            new_set_size = 0;
            bsg_fence();
        });
    if (set_is_dense) {
        static constexpr int STRIDE = BLOCK_WORDS*32;
        parallel_foreach_static(0, V, STRIDE, [=](int v_start){
                int members[STRIDE];
                int members_size = 0;
                int v_stop = v_start + STRIDE;
                for (int v = v_start; v < v_stop; v += 32) {
                    int idx = v/32;
                    int w = set[idx];
                    for (int m = 0; m < 32; m++) {
                        if (w & (1<<m)) {
                            members[members_size++] = v+m;
                        }
                    }
                }
                int start = bsg_amoadd(&new_set_size, members_size);
                if (start >= set_size){
                    pfor_break;
                } else if (members_size > 0) {
                    for (int m = 0; m < members_size; m++) {
                        g_dense_to_sparse_set[start+m] = members[m];
                    }
                }
                return 0;
            });
        serial (
            {
                pr_int_dbg(3000000+set_size);
                pr_int_dbg(4000000+new_set_size);
            }
            );
        bsg_barrier_hw_tile_group_sync();
        return g_dense_to_sparse_set;
    } else {
        bsg_barrier_hw_tile_group_sync();
        return set;
    }
}

static inline int *to_dense(int *set, int set_size, int set_is_dense)
{
    __attribute__((section(".dram"))) static int new_set_size = 0;
    serial({
            new_set_size = 0;
            bsg_fence();
        });
    if (set_is_dense) {
        return set;
    } else {
        zero_dense_set(g_sparse_to_dense_set);
        parallel_foreach_static(0, set_size, 1, [=](int m){
                insert_into_dense(set[m], g_sparse_to_dense_set, &new_set_size);
                return 0;
            });
        bsg_barrier_hw_tile_group_sync();
        return g_sparse_to_dense_set;
    }
}

extern "C"
int kernel()
{
    // used for load balancing
    static __attribute__((section(".dram"))) int g_dst = 0;
    static __attribute__((section(".dram"))) int g_src = 0;

    bsg_barrier_hw_tile_group_init();
    bsg_cuda_print_stat_kernel_start();
    // initialize
    V = g_V;
    E = g_E;
    fwd_offsets = g_fwd_offsets;
    fwd_nonzeros = g_fwd_nonzeros;
    rev_offsets = g_rev_offsets;
    rev_nonzeros = g_rev_nonzeros;
    distance = g_distance;

    serial({
            // setup distance
            distance[g_root] = 0;
            // setup curr_frontier
            g_curr_frontier[0] = g_root;
            g_curr_frontier_size = 1;
            g_curr_frontier_dense = 0;
            // setup next_frontier
            g_next_frontier_dense = 1;
            g_next_frontier_size = 0;
            pr_dbg("root = %d, distance[%d]=%d, curr_fontier[0]=%d, curr_frontier_size=%d\n"
                   ,g_root
                   ,g_root
                   ,distance[g_root]
                   ,g_curr_frontier[0]
                   ,g_curr_frontier_size);
            bsg_fence();
        });
    int d = 0;
    while (g_curr_frontier_size != 0) {
        d++;
        /////////////////////////
        // determine direction //
        /////////////////////////
        int *sparse_frontier = to_sparse(g_curr_frontier, g_curr_frontier_size, g_curr_frontier_dense);
        serial({
                pr_int_dbg(g_curr_frontier_size);
                pr_dbg("curr_frontier_size = %d\n", g_curr_frontier_size);
                g_mf = 0;
                g_mu = 0;
                bsg_fence();
            });
        // 1. find sum (degree in frontier)
        int mf_local = 0;
        parallel_foreach_static(0, g_curr_frontier_size, 1, [=](int m) mutable {
                pr_vdbg("m = %d, sparse_frontier[%d]=%d\n", m, m, sparse_frontier[m]);
                pr_vdbg("fwd_offsets[%d+1]=%d, fwd_offsets[%d]=%d\n",
                        sparse_frontier[m],
                        fwd_offsets[sparse_frontier[m]+1],
                        sparse_frontier[m],
                        fwd_offsets[sparse_frontier[m]]
                    );
                mf_local += fwd_offsets[sparse_frontier[m]+1]-fwd_offsets[sparse_frontier[m]];
                return 0;
            });
        bsg_amoadd(&g_mf, mf_local);
        // 2. find sum (degree unvisited)
        int mu_local;
        parallel_foreach_static(0, V, 1, [=](int v) mutable {
                pr_vdbg("v = %d, distance[%d]=%d\n", v, v, distance[v]);
                if (distance[v] == -1) {
                    pr_vdbg("fwd_offsets[%d+1]=%d, fwd_offsets[%d]=%d\n",
                            v, fwd_offsets[v+1], v, fwd_offsets[v]);
                    mu_local += fwd_offsets[v+1]-fwd_offsets[v];
                }
                return 0;
            });
        bsg_amoadd(&g_mu, mu_local);
        // 3. compare
        serial(
            {
                g_rev_not_fwd = (g_mf > g_mu/20);
                g_dst = 0;
                g_src = 0;
                pr_vdbg("mf = %d, mu = %d, rev_not_fwd = %d\n", g_mf, g_mu, g_rev_not_fwd);
                bsg_fence();
            }
            );
        ////////////////////////////////////
        // traverse from current frontier //
        // update distance                //
        // build next frontier            //
        ////////////////////////////////////
        if (g_rev_not_fwd) {
            int *dense_frontier = to_dense(g_curr_frontier,
                                           g_curr_frontier_size,
                                           g_curr_frontier_dense);
            parallel_foreach_dynamic(g_dst, V, BLOCK_WORDS, [=](int dst_start){
                    int dst_stop = dst_start + BLOCK_WORDS;
                    dst_stop = std::min(dst_stop, V);
                    for (int dst = dst_start; dst < dst_stop; dst++) {
                        // skip if visited already
                        if (distance[dst] == -1) {
                            int nz_start = rev_offsets[dst];
                            int nz_stop  = rev_offsets[dst+1];
                            for (int nz = nz_start; nz < nz_stop; nz++) {
                                int src = rev_nonzeros[nz];
                                pr_int_dbg(2000000+src);
                                if (in_dense(src, dense_frontier)) {
                                    distance[dst] = d;
                                    pr_vdbg("discoverd %d, d=%d\n", dst, d);
                                    pr_int_dbg(1000000+dst);
                                    insert_into_dense(dst, g_next_frontier, &g_next_frontier_size);
                                    break;
                                }
                            }
                        }
                    }
                    return 0;
                });
        } else {
            parallel_foreach_dynamic(g_src, g_curr_frontier_size, 1, [=](int v){
                    int src = sparse_frontier[v];
                    pr_int_dbg(2000000+src);
                    int nz_start = fwd_offsets[src];
                    int nz_stop  = fwd_offsets[src+1];
                    for (int nz = nz_start; nz < nz_stop; nz++) {
                        int dst = fwd_nonzeros[nz];
                        if (distance[dst] == -1) {
                            distance[dst] = d;
                            pr_vdbg("discoverd %d, d=%d\n", dst, d);
                            pr_int_dbg(1000000+dst);
                            insert_into_dense(dst, g_next_frontier, &g_next_frontier_size);
                        }
                    }
                    return  0;
                });
        }
        ////////////////////
        // swap frontiers //
        ////////////////////
        serial(
            {
                // curr = next
                int *tmp = g_curr_frontier;
                g_curr_frontier = g_next_frontier;
                g_curr_frontier_size = g_next_frontier_size;
                g_curr_frontier_dense = g_next_frontier_dense;
                // set next
                g_next_frontier = tmp;
                g_next_frontier_size = 0;
                g_next_frontier_dense = 1;
                // fence
                bsg_fence();
            }
            );
        // zero out next frontier
        zero_dense_set(g_next_frontier);
    }
    bsg_cuda_print_stat_kernel_end();
    bsg_barrier_hw_tile_group_sync();
    return 0;
}
