#include <cello/cello.hpp>
#include <util/statics.hpp>
#include "bs_kernel.hpp"
#include "common.hpp"

DRAM(vector_type) data;
DRAM(OptionData) output;

#ifndef GRAIN_SCALE
#define GRAIN_SCALE 8
#endif

#ifdef TRACE
#define trace(x) \
    bsg_print_int(x)
#else
#define trace(x)
#endif

template <typename T>
using gref = bsg_global_pointer::reference<T>;

using guard = bsg_global_pointer::pod_address_guard;

int cello_main(int argc, char *argv[])
{
    trace(0x1000+0xAAA);
    int grain = data.local_size()/(cello::threads()*GRAIN_SCALE);
    if (grain < 1)
        grain = 1;

#ifdef CELLO_GLOBAL_STEALING
    data.foreach_unrestricted(grain, [](int i, gref<OptionData> dram_data_ref) {
        trace(i);

        OptionData dmem_data;
        OptionData *dram_data = reinterpret_cast<OptionData*>(dram_data_ref.to_local());
        float s0, strike0, r0, v0, t0;
        {
            guard _(dram_data_ref.addr().ext().pod_addr());
            // Copy from DRAM;
            s0      = dram_data->s;
            strike0 = dram_data->strike;
            r0      = dram_data->r;
            v0      = dram_data->v;
            t0      = dram_data->t;
        }

        asm volatile("" ::: "memory");

        dmem_data.s      = s0;
        dmem_data.strike = strike0;
        dmem_data.r      = r0;
        dmem_data.v      = v0;
        dmem_data.t      = t0;

        asm volatile("" ::: "memory");

        // calculate call and put;
        BlkSchlsEqEuroNoDiv_kernel(&dmem_data);

        // write back;
        {
            guard _(dram_data_ref.addr().ext().pod_addr());            
            dram_data->call = dmem_data.call;        
            dram_data->put  = dmem_data.put;
            dram_data->completed = 1;
        }
    });
#else                                        
    data.foreach_block(grain, [](int start, int stop, OptionData *dram_data, [[maybe_unused]] OptionData *_){
        trace(i);

        OptionData dmem_data[CHUNK_SIZE];
        // Copy from DRAM;
        float s0      = dram_data[0].s;
        float strike0 = dram_data[0].strike;
        float r0      = dram_data[0].r;
        float v0      = dram_data[0].v;
        float t0      = dram_data[0].t;

        float s1      = dram_data[1].s;
        float strike1 = dram_data[1].strike;
        float r1      = dram_data[1].r;
        float v1      = dram_data[1].v;
        float t1      = dram_data[1].t;

        float s2      = dram_data[2].s;
        float strike2 = dram_data[2].strike;
        float r2      = dram_data[2].r;
        float v2      = dram_data[2].v;
        float t2      = dram_data[2].t;

        float s3      = dram_data[3].s;
        float strike3 = dram_data[3].strike;
        float r3      = dram_data[3].r;
        float v3      = dram_data[3].v;
        float t3      = dram_data[3].t;        

        asm volatile("" ::: "memory");

        dmem_data[0].s      = s0;
        dmem_data[0].strike = strike0;
        dmem_data[0].r      = r0;
        dmem_data[0].v      = v0;
        dmem_data[0].t      = t0;

        dmem_data[1].s      = s1;
        dmem_data[1].strike = strike1;
        dmem_data[1].r      = r1;
        dmem_data[1].v      = v1;
        dmem_data[1].t      = t1;

        dmem_data[2].s      = s2;
        dmem_data[2].strike = strike2;
        dmem_data[2].r      = r2;
        dmem_data[2].v      = v2;
        dmem_data[2].t      = t2;

        dmem_data[3].s      = s3;
        dmem_data[3].strike = strike3;
        dmem_data[3].r      = r3;
        dmem_data[3].v      = v3;
        dmem_data[3].t      = t3;
       
        asm volatile("" ::: "memory");

        for (int i = 0; i < CHUNK_SIZE; i++) {
            // calculate call and put;
            BlkSchlsEqEuroNoDiv_kernel(&dmem_data[i]);
        }

        // write back;
        dram_data[0].call = dmem_data[0].call;        
        dram_data[0].put  = dmem_data[0].put;
        //dram_data[0].completed = 1;
        dram_data[1].call = dmem_data[1].call;        
        dram_data[1].put  = dmem_data[1].put;
        //dram_data[1].completed = 1;
        dram_data[2].call = dmem_data[2].call;        
        dram_data[2].put  = dmem_data[2].put;
        //dram_data[2].completed = 1;
        dram_data[3].call = dmem_data[3].call;        
        dram_data[3].put  = dmem_data[3].put;
        //dram_data[3].completed = 1;        
    });
#endif
    //output.completed = 77;
    return 0;
}
