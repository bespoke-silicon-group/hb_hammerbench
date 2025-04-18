#include <cello/cello.hpp>
#include <util/statics.hpp>
#include "common256.hpp"

// batch of fft inputs/output
DRAM(batch_vector) in;
DRAM(batch_vector) out;

// twittle factors
DRAM(matrix) twiddle;

DMEM(FP32Complex) fft_workset[NUM_POINTS];

int cello_main(int argc, char *argv[])
{
    in.foreach<cello::serial>([](int i, matrix & in_matrix) {
        matrix &out_matrix = out.local(i);
        cello::foreach<cello::parallel>(0, NUM_POINTS, [&in_matrix, &out_matrix](int col){
            // load_strided
            load_strided(fft_workset, &in_matrix[0][col]);
            //fft256
            fft256_specialized(fft_workset);
            // twiddle scaling
            // scale this column by the matching row in twiddle scaling amtrix
            twiddle_scaling(fft_workset, &twiddle[col][0]);
            // write back
            store_strided(const_cast<FP32Complex*>(&in_matrix[0][col]), fft_workset);
        });

        cello::foreach<cello::parallel>(0, NUM_POINTS, [&in_matrix, &out_matrix](int row){
            // load sequential
            load_sequential(fft_workset, &in_matrix[row][0]);
            // fft256
            fft256_specialized(fft_workset);
            // store strided
            store_strided(const_cast<FP32Complex*>(&out_matrix[0][row]), fft_workset);
        });
    });
    return 0;
}
