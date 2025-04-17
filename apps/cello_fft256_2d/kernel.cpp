#include <cello/cello.hpp>
#include <util/statics.hpp>
#include "common256.hpp"

// batch of fft inputs/output
DRAM(batch_vector) in;
DRAM(batch_vector) out;

// twittle factors
DRAM(matrix) twiddle;

int cello_main(int argc, char *argv[])
{
    in.foreach([](int i, matrix & in_matrix) {
        cello::foreach(0, NUM_POINTS, [](int iter){
            // columns
            unsigned sp;
            asm volatile ("mv %0, sp" : "=r"(sp));
            bsg_print_hexadecimal(sp);
        });

        cello::foreach(0, NUM_POINTS, [](int iter){
            // rows
            unsigned sp;
            asm volatile ("mv %0, sp" : "=r"(sp));
            bsg_print_hexadecimal(sp);            
        });        
    });
    return 0;
}
