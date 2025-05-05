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

static DMEM(global_node) global_tg_node;

DMEM(pointer<global_node>) result_global_node;

static int setup_global_tg()
{
    int idn  = __bsg_id+1;
    int carry = 0;
    if (idn == bsg_tiles_X*bsg_tiles_Y) {
        carry = 1;
        idn = 0;
    }

    int podn = bsg_pod_id+carry;
    if (podn == bsg_pods_X*bsg_pods_Y) {
        podn = 0;
    }

    int pxn, pyn, txn, tyn;
    txn = bsg_id_to_x(idn);
    tyn = bsg_id_to_y(idn);
    pxn = podn % bsg_pods_X;
    pyn = podn / bsg_pods_X;
    global_tg_node.next = pointer<global_node>::onPodXY(pxn, pyn, bsg_tile_group_remote_pointer<global_node>(txn, tyn, &global_tg_node));
    return 0;
}


int setup()
{
    bsg_barrier_tile_group_init();
    bsg_barrier_tile_group_sync();
    setup_global_tg();
    bsg_barrier_tile_group_sync();    
    return 0;
}

static int kernel_global_tg()
{
    pointer<global_node> p = pointer<global_node>(bsg_tile_group_remote_pointer<global_node>(__bsg_x, __bsg_y, &global_tg_node));
    register pod_address save;
    int i = N;
    for (; i != 0; ) {
        register global_node *praw = reinterpret_cast<global_node*>(p.ref().addr().raw());
        register pod_address addr(p.ref().addr().ext().pod_addr());
        pod_address::writePodAddrCSRNoFence(addr);
        p = praw->next;
        asm volatile ("addi %0, %1, -1" : "=r"(i) : "r"(i) : "memory");
    }
    pod_address::writePodAddrCSR(save);
    result_global_node = p;
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
        kernel_global_tg();
    }
    bsg_cuda_print_stat_kernel_end();
    bsg_fence();
    bsg_barrier_tile_group_sync();
    return 0;
}
