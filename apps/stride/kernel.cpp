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
DMEM(pointer<global_node>) result_global_tg_node;
static DMEM(node) tg_node;
DMEM(node*) result_tg_node;

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
#ifdef GLOBAL
    setup_global_tg();
#else
    setup_tg();
#endif
    bsg_barrier_tile_group_sync();    
    return 0;
}

static int kernel_global_tg()
{
    pointer<global_node> p = pointer<global_node>(bsg_tile_group_remote_pointer<global_node>(__bsg_x, __bsg_y, &global_tg_node));
    for (int i = 0; i < N; i++) {
        register global_node *praw = reinterpret_cast<global_node*>(p.ref().addr().raw());
        pod_address_guard _ (p.ref().addr().ext().pod_addr());
        {
            p = praw->next;
        }
    }
    result_global_tg_node = p;
    return 0;
}

static int kernel_tg()
{    
    node *p = &tg_node;
    for (int i = 0; i < N; i++) {
        p = p->next;
    }
    result_tg_node = p;
    return 0;
}

int kernel()
{
    bsg_barrier_tile_group_init();
    bsg_barrier_tile_group_sync();
    bsg_cuda_print_stat_kernel_start();
    if (__bsg_x == TILE_X &&
        __bsg_y == TILE_Y &&
        __bsg_pod_x == POD_X &&
        __bsg_pod_y == POD_Y) {        
#ifdef GLOBAL
        kernel_global_tg();
#else
        kernel_tg();
#endif
    }
    bsg_cuda_print_stat_kernel_end();
    bsg_barrier_tile_group_sync();
    return 0;
}
