// 256x256 FFT using four-step method
// In this implementation all tiles write back the intermediate 256-point
// FFT results to the DRAM

#include <bsg_manycore.h>
#include <bsg_set_tile_x_y.h>
#include <bsg_cuda_lite_barrier.h>

#include <fft256.hpp>


// Cache warming function.
// Can only fit A and TW in the cache.
#ifdef WARM_CACHE
#define CACHE_LINE_COMPLEX 8
__attribute__ ((noinline))
static int warmup(FP32Complex *in, FP32Complex *out, float bsg_attr_remote * bsg_attr_noalias tw, int N, int num_iter)
{
  for (int i = __bsg_id*CACHE_LINE_COMPLEX; i < N*num_iter; i += bsg_tiles_X*bsg_tiles_Y*CACHE_LINE_COMPLEX) {
      asm volatile ("lw x0, %[p]" :: [p] "m" (in[i]));
      //asm volatile ("lw x0, %[p]" :: [p] "m" (out[i]));
  }
  for (int i = __bsg_id*CACHE_LINE_COMPLEX; i < N; i += bsg_tiles_X*bsg_tiles_Y*CACHE_LINE_COMPLEX) {
      asm volatile ("lw x0, %[p]" :: [p] "m" (tw[i]));
  }
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();
  return 0;
}
#endif



FP32Complex fft_workset[NUM_POINTS];

extern "C" __attribute__ ((noinline))
int
kernel_fft(FP32Complex * bsg_attr_noalias  in,
           FP32Complex * bsg_attr_noalias out,
           float bsg_attr_remote * bsg_attr_noalias tw,
           int N,
           int num_iter
            ) {

    bsg_barrier_hw_tile_group_init();
    bsg_fence();

    #ifdef WARM_CACHE
    warmup(in, out, tw, N, num_iter);
    #endif

    bsg_cuda_print_stat_kernel_start();

    int stat_count = 1;
    for (int i = 0; i < num_iter; i++) {
      // Step 1
      for (int iter = 0; iter < 2; iter++) {
        load_fft_store_no_twiddle(&in[i*N], &in[i*N], fft_workset, tw, iter*128+__bsg_id, 256, 256, N, 1);
      }
      asm volatile("": : :"memory");
      bsg_barrier_hw_tile_group_sync();

      // Step 2
      opt_square_transpose(&in[i*N], 256);
      asm volatile("": : :"memory");
      bsg_barrier_hw_tile_group_sync();


      // step 3
      for (int iter = 0; iter < 2; iter++) {
        load_fft_store_no_twiddle(&in[i*N], &out[i*N], fft_workset, tw, iter*128+__bsg_id, 256, 256, N, 0);
      }
      bsg_barrier_hw_tile_group_sync();
    }


    // Kernel end
    bsg_cuda_print_stat_kernel_end();
    bsg_fence();
    bsg_barrier_hw_tile_group_sync();



    return 0;
}
