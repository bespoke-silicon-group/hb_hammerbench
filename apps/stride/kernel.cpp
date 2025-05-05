#include <standard/standard.hpp>
#include <global_pointer/global_pointer.hpp>
#include <util/statics.hpp>
#include <bsg_tile_config_vars.h>
#include <bsg_manycore.hpp>
#include <bsg_cuda_lite_barrier.h>

struct node {
    node *next;
};


DMEM(node*) result_node;

DRAM(node) *dram_node;

static int setup_dram()
{
    for (int i = __bsg_id; i < N; i += bsg_tiles_X*bsg_tiles_Y) {
        int in = i+1;
        int j = VECTOR_SIZE > 0 ? (i * STRIDE_SIZE) % VECTOR_SIZE : 0;
        int jn = VECTOR_SIZE > 0 ? (in * STRIDE_SIZE) % VECTOR_SIZE : 0;
        dram_node[j].next = &dram_node[jn];
    }
    return 0;
}


int setup()
{
    bsg_barrier_tile_group_init();
    bsg_barrier_tile_group_sync();
    setup_dram();
    bsg_barrier_tile_group_sync();    
    return 0;
}


static int kernel_dram()
{
    node *p = &dram_node[0];
    for (int i = 0; i < N; i++) {
        p = p->next;
    }
    result_node = p;
    return 0;
}


int kernel()
{
    bsg_barrier_tile_group_init();   
    int pod_x = __bsg_pod_x;
    int pod_y = __bsg_pod_y;
    int tile_x = __bsg_x;
    int tile_y = __bsg_y;
    bsg_fence();
    bsg_barrier_tile_group_sync();    
    bsg_cuda_print_stat_kernel_start();
    if (tile_x == TILE_X &&
        tile_y == TILE_Y &&
        pod_x == POD_X &&
        pod_y == POD_Y) {        
        kernel_dram();
    }
    bsg_cuda_print_stat_kernel_end();
    bsg_fence();
    bsg_barrier_tile_group_sync();
    return 0;
}
