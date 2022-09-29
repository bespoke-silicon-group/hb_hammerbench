#include "bsg_manycore.h"
#include "bsg_manycore_atomic.h"
#include "bsg_cuda_lite_barrier.h"
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

//#define VDEBUG
#ifdef  VDEBUG
#define pr_dbg(fmt, ...)                        \
    bsg_printf(fmt, ##__VA_ARGS__)
#else
#define pr_dbg(fmt, ...)
#endif

static inline void zero_dense_set(int *set)
{
    bsg_barrier_hw_tile_group_sync();
    static constexpr int STRIDE = 32*BLOCK_WORDS;
    for (int v_start = __bsg_id*STRIDE; v_start < V; v_start += STRIDE*bsg_tiles_X*bsg_tiles_Y) {
        int v_stop = v_start + STRIDE;
        for (int v = v_start; v < v_stop; v += 32) {
            int idx = v/32;
            set[idx] = 0;
        }
    }
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
        int members[STRIDE];
        int members_size = 0;
        for (int v_start = __bsg_id*STRIDE; v_start < V; v_start += bsg_tiles_X*bsg_tiles_Y) {
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
                break;
            } else if (members_size > 0) {
                for (int m = 0; m < members_size; m++) {
                    g_dense_to_sparse_set[start+m] = members[m];
                }
                members_size = 0;
            }
        }
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
        for (int m = __bsg_id; m < set_size; m += bsg_tiles_X*bsg_tiles_Y) {
            insert_into_dense(set[m], g_sparse_to_dense_set, &new_set_size);
        }
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
        for (int m = __bsg_id; m < g_curr_frontier_size; m += bsg_tiles_X*bsg_tiles_Y) {
            // pr_dbg("m = %d, sparse_frontier[%d]=%d\n", m, m, sparse_frontier[m]);
            // pr_dbg("fwd_offsets[%d+1]=%d, fwd_offsets[%d]=%d\n",
            //        sparse_frontier[m],
            //        fwd_offsets[sparse_frontier[m]+1],
            //        sparse_frontier[m],
            //        fwd_offsets[sparse_frontier[m]]
            //     );
            mf_local += fwd_offsets[sparse_frontier[m]+1]-fwd_offsets[sparse_frontier[m]];
        }
        bsg_amoadd(&g_mf, mf_local);
        // 2. find sum (degree unvisited)
        int mu_local;
        for (int v = __bsg_id; v < V; v += bsg_tiles_X*bsg_tiles_Y) {
            // pr_dbg("v = %d, distance[%d]=%d\n", v, v, distance[v]);
            if (distance[v] == -1) {
                // pr_dbg("fwd_offsets[%d+1]=%d, fwd_offsets[%d]=%d\n",
                //        v, fwd_offsets[v+1], v, fwd_offsets[v]);
                mu_local += fwd_offsets[v+1]-fwd_offsets[v];
            }
        }
        bsg_amoadd(&g_mu, mu_local);
        // 3. compare
        serial(
            {                
                g_rev_not_fwd = (g_mf > g_mu/20);
                g_dst = 0;
                g_src = 0;
                pr_dbg("mf = %d, mu = %d, rev_not_fwd = %d\n", g_mf, g_mu, g_rev_not_fwd);
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
            for (int dst_start = bsg_amoadd(&g_dst, BLOCK_WORDS);
                 dst_start < V;
                 dst_start = bsg_amoadd(&g_dst, BLOCK_WORDS)) {
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
                                pr_dbg("discoverd %d, d=%d\n", dst, d);
                                pr_int_dbg(1000000+dst);
                                insert_into_dense(dst, g_next_frontier, &g_next_frontier_size);
                                break;
                            }
                        }
                    }
                }
            }
        } else {
            for (int v = bsg_amoadd(&g_src, 1); v < g_curr_frontier_size; v = bsg_amoadd(&g_src, 1)) {
                int src = sparse_frontier[v];
                pr_int_dbg(2000000+src);
                int nz_start = fwd_offsets[src];
                int nz_stop  = fwd_offsets[src+1];
                for (int nz = nz_start; nz < nz_stop; nz++) {
                    int dst = fwd_nonzeros[nz];
                    if (distance[dst] == -1) {
                        distance[dst] = d;
                        pr_dbg("discoverd %d, d=%d\n", dst, d);
                        pr_int_dbg(1000000+dst);                        
                        insert_into_dense(dst, g_next_frontier, &g_next_frontier_size);
                    }
                }
            }
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
