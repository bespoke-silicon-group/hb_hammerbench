#include <cello/cello.hpp>
#include <util/statics.hpp>
#include "common256.hpp"

// batch of fft inputs/output
DRAM(batch_vector) in;
DRAM(batch_vector) out;

// twittle factors
DRAM(matrix) twiddle;

DMEM(FP32Complex) fft_workset[NUM_POINTS];

#define GRAIN (NUM_POINTS/(2*cello::threads()))

template <typename T>
using gref = bsg_global_pointer::reference<T>;
template <typename T>
using gptr = bsg_global_pointer::pointer<T>;
using guard = bsg_global_pointer::pod_address_guard;

int cello_main(int argc, char *argv[])
{
#ifdef CELLO_GLOBAL_STEALING
    in.foreach_unrestricted<cello::serial>([](int i, gref<matrix> in_matrix) {
        gref<matrix> out_matrix = out.at(i);
        cello::foreach<cello::parallel>(0, NUM_POINTS, 1, GRAIN, [in_matrix, out_matrix](int col){
            {
                matrix &in = *reinterpret_cast<matrix*>(in_matrix.to_local());
                guard _ (in_matrix.addr().ext().pod_addr());
                load_strided(fft_workset, &in[0][col]);
            }
            //fft256
            fft256_specialized(fft_workset);
            // twiddle scaling
            // scale this column by the matching row in twiddle scaling amtrix
            twiddle_scaling(fft_workset, &twiddle[col][0]);
            {
                matrix &in = *reinterpret_cast<matrix*>(in_matrix.to_local());
                guard _ (in_matrix.addr().ext().pod_addr());
                store_strided(const_cast<FP32Complex*>(&in[0][col]), fft_workset);
            }
        });
        cello::foreach<cello::parallel>(0, NUM_POINTS, 1, GRAIN, [in_matrix, out_matrix](int row){
            // load sequential
            {
                matrix &in = *reinterpret_cast<matrix*>(in_matrix.to_local());
                guard _ (in_matrix.addr().ext().pod_addr());
                load_sequential(fft_workset, &in[row][0]);
            }
            // fft256
            fft256_specialized(fft_workset);
            {
                matrix &out = *reinterpret_cast<matrix*>(out_matrix.to_local());
                guard _ (out_matrix.addr().ext().pod_addr());
                store_strided(const_cast<FP32Complex*>(&out[0][row]), fft_workset);
            }
        });
    });
#else
    in.foreach<cello::serial>([](int i, matrix & in_matrix) {
        matrix & out_matrix = out.local(i);
        cello::foreach<cello::parallel>(0, NUM_POINTS, 1, GRAIN, [&in_matrix, &out_matrix](int col){
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
        cello::foreach<cello::parallel>(0, NUM_POINTS, 1, GRAIN, [&in_matrix, &out_matrix](int row){
            // load sequential
            load_sequential(fft_workset, &in_matrix[row][0]);
            // fft256
            fft256_specialized(fft_workset);
            // store strided
            store_strided(const_cast<FP32Complex*>(&out_matrix[0][row]), fft_workset);
        });
    });
#endif
    return 0;
}
