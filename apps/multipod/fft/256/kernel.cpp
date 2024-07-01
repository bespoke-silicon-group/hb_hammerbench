#include <bsg_manycore.h>
#include <bsg_cuda_lite_barrier.h>
#include "fft256.hpp"
#include "bsg_barrier_multipod.h"

#define N (NUM_POINTS*NUM_POINTS)
#define NUM_TILES (bsg_tiles_X*bsg_tiles_Y)




// Local storage;
FP32Complex fft_workset[NUM_POINTS];


// Kernel main;
extern "C" int
kernel(FP32Complex * in,
           FP32Complex * out,
           FP32Complex * tw,
           int num_iter, // how many of batches of fft?
           int pod_id)
{
    bsg_barrier_hw_tile_group_init();
    bsg_fence();
    bsg_barrier_hw_tile_group_sync();

    // remapping tile id;
    int my_id = (__bsg_id % NUM_POINTS);
    int group_id = (__bsg_id / NUM_POINTS);
    int num_group = (NUM_TILES / NUM_POINTS);


    // Kernel start;
    bsg_barrier_hw_tile_group_sync();
    bsg_cuda_print_stat_kernel_start();

    for (int i = 0; i < num_iter; i+=num_group) {
      FP32Complex *input_sq  = &in[(i+group_id)*N];
      FP32Complex* input_vec = input_sq + my_id;
      // step 1;
      load_strided(fft_workset, input_vec);
      fft256_specialized(fft_workset);
      twiddle_scaling(fft_workset, tw+(my_id*NUM_POINTS));
      store_strided(input_vec, fft_workset);
      asm volatile("": : :"memory");
      bsg_fence();
      bsg_barrier_hw_tile_group_sync();



      // step 2
      input_vec = input_sq + (my_id * NUM_POINTS);
      FP32Complex *output_sq = &out[(i+group_id)*N];
      FP32Complex* output_vec = output_sq + my_id;
      load_sequential(fft_workset, input_vec);
      fft256_specialized(fft_workset);
      store_strided(output_vec, fft_workset);
      bsg_fence();
      bsg_barrier_hw_tile_group_sync();
    }


    // Kernel end
    bsg_cuda_print_stat_kernel_end();
    bsg_fence();
    bsg_barrier_hw_tile_group_sync();

    return 0;
}
