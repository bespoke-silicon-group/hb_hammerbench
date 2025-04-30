#include <standard/standard.hpp>
#include <bsg_cuda_lite_barrier.h>
#include <util/statics.hpp>
#include <bsg_manycore.h>

#define SEEDS_PER_THREAD (UPDATES_PER_THREAD > 512 ? 512 : UPDATES_PER_THREAD)

DRAM(unsigned *) g_table;
DRAM(unsigned *) g_seeds;
DMEM(unsigned) l_seeds [SEEDS_PER_THREAD];

int setup()
{
    bsg_barrier_tile_group_init();
    bsg_barrier_tile_group_sync();
    unsigned *seeds_p = &g_seeds[SEEDS_PER_THREAD*__bsg_id];
    int i = 0;
    for (; i+8 <= SEEDS_PER_THREAD; i += 8) {
        register unsigned k[8];
        k[0] = seeds_p[i+0];
        k[1] = seeds_p[i+1];
        k[2] = seeds_p[i+2];
        k[3] = seeds_p[i+3];
        k[4] = seeds_p[i+4];
        k[5] = seeds_p[i+5];
        k[6] = seeds_p[i+6];
        k[7] = seeds_p[i+7];
        asm volatile ("" ::: "memory");
        l_seeds[i+0] = k[0];
        l_seeds[i+1] = k[1];
        l_seeds[i+2] = k[2];
        l_seeds[i+3] = k[3];
        l_seeds[i+4] = k[4];
        l_seeds[i+5] = k[5];
        l_seeds[i+6] = k[6];
        l_seeds[i+7] = k[7];
    }
    for (int i = 0; i < SEEDS_PER_THREAD; i++) {
        l_seeds[i] = seeds_p[i];
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
    register unsigned a = 1664525, c = 1013904223;
#define UNROLL_4
    for (int j; j < UPDATES_PER_THREAD; j += SEEDS_PER_THREAD) {
        int i = 0;
#ifdef UNROLL_8   
        for (; i+8 <= SEEDS_PER_THREAD; i += 8) {
            register unsigned k [8];
            register unsigned v [8];
            register unsigned s [8];
            s[0] = l_seeds[i+0];
            s[1] = l_seeds[i+1];
            s[2] = l_seeds[i+2];
            s[3] = l_seeds[i+3];
            s[4] = l_seeds[i+4];
            s[5] = l_seeds[i+5];
            s[6] = l_seeds[i+6];
            s[7] = l_seeds[i+7];            
            k[0] = s[0] & (TABLE_SIZE-1);
            k[1] = s[1] & (TABLE_SIZE-1);
            k[2] = s[2] & (TABLE_SIZE-1);
            k[3] = s[3] & (TABLE_SIZE-1);
            k[4] = s[4] & (TABLE_SIZE-1);
            k[5] = s[5] & (TABLE_SIZE-1);
            k[6] = s[6] & (TABLE_SIZE-1);
            k[7] = s[7] & (TABLE_SIZE-1);
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
            s[0] = s[0] * a;
            s[1] = s[1] * a;
            s[2] = s[2] * a;
            s[3] = s[3] * a;
            s[4] = s[4] * a;
            s[5] = s[5] * a;
            s[6] = s[6] * a;
            s[7] = s[7] * a;

            s[0] = s[0] + c;
            s[1] = s[1] + c;
            s[2] = s[2] + c;
            s[3] = s[3] + c;
            s[4] = s[4] + c;
            s[5] = s[5] + c;
            s[6] = s[6] + c;
            s[7] = s[7] + c;

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
            asm volatile("" ::: "memory");        
            l_seeds[i+0] = s[0];
            l_seeds[i+1] = s[1];
            l_seeds[i+2] = s[2];
            l_seeds[i+3] = s[3];
            l_seeds[i+4] = s[4];
            l_seeds[i+5] = s[5];
            l_seeds[i+6] = s[6];
            l_seeds[i+7] = s[7];
            bsg_fence();        
        }
#elif defined(UNROLL_4)
        for (; i+4 <= SEEDS_PER_THREAD; i += 4) {
            register unsigned k [4];
            register unsigned v [4];
            register unsigned s [4];
            s[0] = l_seeds[i+0];
            s[1] = l_seeds[i+1];
            s[2] = l_seeds[i+2];
            s[3] = l_seeds[i+3];
            k[0] = s[0] & (TABLE_SIZE-1);
            k[1] = s[1] & (TABLE_SIZE-1);
            k[2] = s[2] & (TABLE_SIZE-1);
            k[3] = s[3] & (TABLE_SIZE-1);
            asm volatile ("" ::: "memory");
            v[0] = l_table[k[0]];
            v[1] = l_table[k[1]];
            v[2] = l_table[k[2]];
            v[3] = l_table[k[3]];
            asm volatile("" ::: "memory");
            s[0] = s[0] * a;
            s[1] = s[1] * a;
            s[2] = s[2] * a;
            s[3] = s[3] * a;

            s[0] = s[0] + c;
            s[1] = s[1] + c;
            s[2] = s[2] + c;
            s[3] = s[3] + c;

            v[0] ^= k[0];
            v[1] ^= k[1];
            v[2] ^= k[2];
            v[3] ^= k[3];            
            asm volatile("" ::: "memory");
            l_table[k[0]] = v[0];
            l_table[k[1]] = v[1];
            l_table[k[2]] = v[2];
            l_table[k[3]] = v[3];
            asm volatile("" ::: "memory");        
            l_seeds[i+0] = s[0];
            l_seeds[i+1] = s[1];
            l_seeds[i+2] = s[2];
            l_seeds[i+3] = s[3];
            bsg_fence();        
        }
#elif defined(UNROLL_2)
        for (; i+2 <= SEEDS_PER_THREAD; i += 2) {
            register unsigned k [2];
            register unsigned v [2];
            register unsigned s [2];
            s[0] = l_seeds[i+0];
            s[1] = l_seeds[i+1];
            k[0] = s[0] & (TABLE_SIZE-1);
            k[1] = s[1] & (TABLE_SIZE-1);
            asm volatile ("" ::: "memory");
            v[0] = l_table[k[0]];
            v[1] = l_table[k[1]];
            asm volatile("" ::: "memory");
            s[0] = s[0] * a;
            s[1] = s[1] * a;

            s[0] = s[0] + c;
            s[1] = s[1] + c;

            v[0] ^= k[0];
            v[1] ^= k[1];
            asm volatile("" ::: "memory");
            l_table[k[0]] = v[0];
            l_table[k[1]] = v[1];
            asm volatile("" ::: "memory");        
            l_seeds[i+0] = s[0];
            l_seeds[i+1] = s[1];
            bsg_fence();        
        }
#endif
        for (; i < SEEDS_PER_THREAD; i++) {
            unsigned s = l_seeds[i];
            unsigned k = s & (TABLE_SIZE-1);
            unsigned v = l_table[k];
            v = v ^ k;
            l_table[k] = v;
            s = s * a + c;
            bsg_fence();
        }
    }
    bsg_barrier_tile_group_sync();
    bsg_cuda_print_stat_kernel_end();
    bsg_fence();
    bsg_barrier_tile_group_sync();    
    return 0;
}
