#include "bsg_manycore.h"
#include "bsg_manycore_atomic.h"
#include "bsg_cuda_lite_barrier.h"
#include <algorithm>
__attribute__((section(".dram")))
int root;

__attribute__((section(".dram")))
int V;
__attribute__((section(".dram")))
int E;

__attribute__((section(".dram")))
const int *fwd_offsets;
__attribute__((section(".dram")))
const int *fwd_nonzeros;

__attribute__((section(".dram")))
const int *rev_offsets;
__attribute__((section(".dram")))
const int *rev_nonzeros;

__attribute__((section(".dram")))
int *distance;

__attribute__((section(".dram")))
int *curr_frontier;
__attribute__((section(".dram")))
int  curr_frontier_size = 0;
__attribute__((section(".dram")))
int  curr_frontier_dense = 0;

__attribute__((section(".dram")))
int *next_frontier;
__attribute__((section(".dram")))
int  next_frontier_size = 0;
__attribute__((section(".dram")))
int  next_frontier_dense = 1;

__attribute__((section(".dram")))
int rev_not_fwd = 0;

__attribute__((section(".dram")))
int *dense_to_sparse_set;

__attribute__((section(".dram")))
int *sparse_to_dense_set;

__attribute__((section(".dram")))
int mu = 0;
__attribute__((section(".dram")))
int mf = 0;

#define serial(block)                           \
    do {                                        \
        bsg_barrier_hw_tile_group_sync();       \
        if (__bsg_id == 0) {                    \
            block;                              \
        }                                       \
        bsg_fence();                            \
        bsg_barrier_hw_tile_group_sync();       \
    } while (0)

//#define DEBUG
#ifdef  DEBUG
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
                    dense_to_sparse_set[start+m] = members[m];
                }
                members_size = 0;
            }
        }
        serial (
            {
                bsg_print_int(3000000+set_size);
                bsg_print_int(4000000+new_set_size);
            }
            );
        bsg_barrier_hw_tile_group_sync();
        return dense_to_sparse_set;
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
        zero_dense_set(sparse_to_dense_set);
        for (int m = __bsg_id; m < set_size; m += bsg_tiles_X*bsg_tiles_Y) {
            insert_into_dense(set[m], sparse_to_dense_set, &new_set_size);
        }
        bsg_barrier_hw_tile_group_sync();
        return sparse_to_dense_set;
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
    serial({
            // setup distance
            distance[root] = 0;
            // setup curr_frontier
            curr_frontier[0] = root;
            curr_frontier_size = 1;
            curr_frontier_dense = 0;
            // setup next_frontier
            next_frontier_dense = 1;
            next_frontier_size = 0;
            bsg_fence();
        });
    serial({
            pr_dbg("root = %d, distance[%d]=%d, curr_fontier[0]=%d, curr_frontier_size=%d\n"
                   ,root
                   ,root
                   ,distance[root]
                   ,curr_frontier[0]
                   ,curr_frontier_size);
        });
    int d = 0;
    while (curr_frontier_size != 0) {
        serial (
            {
                bsg_print_int(curr_frontier_size);
                pr_dbg("curr_frontier_size = %d\n", curr_frontier_size);
            }
            );
        d++;
        /////////////////////////
        // determine direction //
        /////////////////////////
        bsg_barrier_hw_tile_group_sync();
        int *sparse_frontier = to_sparse(curr_frontier, curr_frontier_size, curr_frontier_dense);
        serial({
                mf = 0;
                mu = 0;
                bsg_fence();
            }); 
        // 1. find sum (degree in frontier)
        serial ( bsg_print_hexadecimal(0xaaaaaaaa) );
        int mf_local = 0;
        for (int m = __bsg_id; m < curr_frontier_size; m += bsg_tiles_X*bsg_tiles_Y) {
            // pr_dbg("m = %d, sparse_frontier[%d]=%d\n", m, m, sparse_frontier[m]);
            // pr_dbg("fwd_offsets[%d+1]=%d, fwd_offsets[%d]=%d\n",
            //        sparse_frontier[m],
            //        fwd_offsets[sparse_frontier[m]+1],
            //        sparse_frontier[m],
            //        fwd_offsets[sparse_frontier[m]]
            //     );
            mf_local += fwd_offsets[sparse_frontier[m]+1]-fwd_offsets[sparse_frontier[m]];
        }
        bsg_amoadd(&mf, mf_local);
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
        bsg_amoadd(&mu, mu_local);
        // 3. compare
        serial(bsg_print_hexadecimal(0xbbbbbbbb));
        serial(
            {                
                rev_not_fwd = (mf > mu/20);
                g_dst = 0;
                g_src = 0;
                pr_dbg("mf = %d, mu = %d, rev_not_fwd = %d\n", mf, mu, rev_not_fwd);
                bsg_fence();
            }
            );
        ////////////////////////////////////
        // traverse from current frontier //
        // update distance                //
        // build next frontier            //
        ////////////////////////////////////
        bsg_barrier_hw_tile_group_sync();
        if (rev_not_fwd) {
            serial (bsg_print_hexadecimal(0xcccc0000));
            serial ({ pr_dbg("bottom up\n"); });
            int *dense_frontier = to_dense(curr_frontier, curr_frontier_size, curr_frontier_dense);
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
                            bsg_print_int(2000000+src);                            
                            if (in_dense(src, dense_frontier)) {
                                distance[dst] = d;
                                pr_dbg("discoverd %d, d=%d\n", dst, d);
                                bsg_print_int(1000000+dst);
                                insert_into_dense(dst, next_frontier, &next_frontier_size);
                                break;
                            }
                        }
                    }
                }
            }
        } else {
            serial (bsg_print_hexadecimal(0xcccc1111) );
            serial ({ pr_dbg("top down\n"); });
            for (int v = bsg_amoadd(&g_src, 1); v < curr_frontier_size; v = bsg_amoadd(&g_src, 1)) {
                int src = sparse_frontier[v];
                bsg_print_int(2000000+src);
                int nz_start = fwd_offsets[src];
                int nz_stop  = fwd_offsets[src+1];
                for (int nz = nz_start; nz < nz_stop; nz++) {
                    int dst = fwd_nonzeros[nz];
                    if (distance[dst] == -1) {
                        distance[dst] = d;
                        pr_dbg("discoverd %d, d=%d\n", dst, d);
                        bsg_print_int(1000000+dst);                        
                        insert_into_dense(dst, next_frontier, &next_frontier_size);
                    }
                }
            }
        }
        ////////////////////
        // swap frontiers //
        ////////////////////
        serial (bsg_print_hexadecimal(0xdddddddd));
        serial(
            {
                // curr = next
                int *tmp = curr_frontier;
                curr_frontier = next_frontier;
                curr_frontier_size = next_frontier_size;
                curr_frontier_dense = next_frontier_dense;
                // set next
                next_frontier = tmp;
                next_frontier_size = 0;
                next_frontier_dense = 1;
                // fence
                bsg_fence();
            }
            );
        // zero out next frontier
        zero_dense_set(next_frontier);
        serial (bsg_print_hexadecimal(0xeeeeeeee));
        bsg_barrier_hw_tile_group_sync();
    }
    bsg_cuda_print_stat_kernel_end();
    bsg_barrier_hw_tile_group_sync();
    return 0;
}
