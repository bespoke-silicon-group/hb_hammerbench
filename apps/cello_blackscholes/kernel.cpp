#include <cello/cello.hpp>
#include <util/statics.hpp>
#include "bs_kernel.hpp"
#include "common.hpp"

DRAM(vector_type) data;
DRAM(OptionData) output;

int cello_main(int argc, char *argv[])
{
#ifdef TRACE
    bsg_print_hexadecimal(0x1000+0xAAA);
#endif
    data.foreach([](int i, OptionData &dram_data){
#ifdef TRACE
        bsg_print_int(i);
#endif
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

    //output.completed = 77;
    return 0;
}
