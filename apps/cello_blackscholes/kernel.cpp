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
    data.foreach(grain, [](int i, OptionData &dram_data){
        trace(i);

        OptionData dmem_data;
        // Copy from DRAM;
        float s0      = dram_data.s;
        float strike0 = dram_data.strike;
        float r0      = dram_data.r;
        float v0      = dram_data.v;
        float t0      = dram_data.t;

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
        dram_data.call = dmem_data.call;        
        dram_data.put  = dmem_data.put;
        dram_data.completed = 1;
    });
#endif
    //output.completed = 77;
    return 0;
}
