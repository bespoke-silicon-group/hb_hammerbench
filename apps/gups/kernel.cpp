#include <standard/standard.hpp>
#include <bsg_cuda_lite_barrier.h>
#include <util/statics.hpp>
#include <bsg_manycore.h>

DRAM(unsigned *) g_table;
DRAM(unsigned *) g_updates;
DMEM(unsigned) l_updates [UPDATES_PER_THREAD];

int setup()
{
    bsg_barrier_tile_group_init();
    bsg_barrier_tile_group_sync();
    unsigned *updates_p = &g_updates[UPDATES_PER_THREAD*__bsg_id];
    int i = 0;
    for (; i+8 <= UPDATES_PER_THREAD; i += 8) {
        register unsigned k[8];
        k[0] = updates_p[i+0];
        k[1] = updates_p[i+1];
        k[2] = updates_p[i+2];
        k[3] = updates_p[i+3];
        k[4] = updates_p[i+4];
        k[5] = updates_p[i+5];
        k[6] = updates_p[i+6];
        k[7] = updates_p[i+7];
        asm volatile ("" ::: "memory");
        l_updates[i+0] = k[0];
        l_updates[i+1] = k[1];
        l_updates[i+2] = k[2];
        l_updates[i+3] = k[3];
        l_updates[i+4] = k[4];
        l_updates[i+5] = k[5];
        l_updates[i+6] = k[6];
        l_updates[i+7] = k[7];
    }
    for (int i = 0; i < UPDATES_PER_THREAD; i++) {
        l_updates[i] = updates_p[i];
    }
    bsg_barrier_tile_group_sync();
    return 0;
}

int kernel()
{
    bsg_barrier_tile_group_init();
    bsg_barrier_tile_group_sync();
    bsg_cuda_print_stat_kernel_start();
    unsigned *l_table = g_table;
    int i = 0;
    for (; i+8 <= UPDATES_PER_THREAD; i += 8) {
        register unsigned k[8];
        register unsigned v[8];
        k[0] = l_updates[i+0];
        k[1] = l_updates[i+1];
        k[2] = l_updates[i+2];
        k[3] = l_updates[i+3];
        k[4] = l_updates[i+4];
        k[5] = l_updates[i+5];
        k[6] = l_updates[i+6];
        k[7] = l_updates[i+7];
        asm volatile ("" ::: "memory");
        v[0] = l_table[k[0]];
        v[1] = l_table[k[1]];
        v[2] = l_table[k[2]];
        v[3] = l_table[k[3]];
        v[4] = l_table[k[4]];
        v[5] = l_table[k[5]];
        v[6] = l_table[k[6]];
        v[7] = l_table[k[7]];
        asm volatile("" ::: "memory");
        v[0] ^= k[0];
        v[1] ^= k[1];
        v[2] ^= k[2];
        v[3] ^= k[3];
        v[4] ^= k[4];
        v[5] ^= k[5];
        v[6] ^= k[6];
        v[7] ^= k[7];
        asm volatile("" ::: "memory");
        l_table[k[0]] = v[0];
        l_table[k[1]] = v[1];
        l_table[k[2]] = v[2];
        l_table[k[3]] = v[3];
        l_table[k[4]] = v[4];
        l_table[k[5]] = v[5];
        l_table[k[6]] = v[6];
        l_table[k[7]] = v[7];
    }
    for (; i < UPDATES_PER_THREAD; i++) {
        unsigned k = l_updates[i];
        unsigned v = l_table[k];
        v = v ^ k;
        l_table[k] = v;
    }
    bsg_barrier_tile_group_sync();
    bsg_cuda_print_stat_kernel_end();
    bsg_fence();
    bsg_barrier_tile_group_sync();    
    return 0;
}
