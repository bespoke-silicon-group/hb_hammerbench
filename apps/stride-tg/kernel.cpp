#include <standard/standard.hpp>
#include <global_pointer/global_pointer.hpp>
#include <util/statics.hpp>
#include <bsg_tile_config_vars.h>
#include <bsg_manycore.hpp>
#include <bsg_cuda_lite_barrier.h>

using bsg_global_pointer::pointer;
using bsg_global_pointer::reference;
using bsg_global_pointer::address_ext;
using bsg_global_pointer::pod_address;
using bsg_global_pointer::pod_address_guard;

struct global_node;
    
struct global_node {
    pointer<global_node> next;
};

struct node {
    node *next;
};

static DMEM(global_node) global_tg_node;
DMEM(pointer<global_node>) result_global_node;

static DRAM(global_node) *global_dram_node;


static DMEM(node) tg_node;
DMEM(node*) result_node;

DRAM(node) *dram_node;

static int setup_tg()
{
    int idn  = __bsg_id+1;
    if (idn == bsg_tiles_X*bsg_tiles_Y) {
        idn = 0;
    }

    int txn, tyn;
    txn = bsg_id_to_x(idn);
    tyn = bsg_id_to_y(idn);

    tg_node.next = bsg_tile_group_remote_pointer<node>(txn, tyn, &tg_node);
    return 0;
}
int setup()
{
    bsg_barrier_tile_group_init();
    bsg_barrier_tile_group_sync();
    setup_tg();
    bsg_barrier_tile_group_sync();    
    return 0;
}


static int kernel_tg()
{    
    node *p = &tg_node;
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
        kernel_tg();
    }
    bsg_cuda_print_stat_kernel_end();
    bsg_fence();
    bsg_barrier_tile_group_sync();
    return 0;
}
