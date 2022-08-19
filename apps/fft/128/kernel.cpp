// 128x128 FFT using four-step method
// In this implementation all tiles communicate through scratchpad stores

#include <bsg_manycore.h>
#include <bsg_cuda_lite_barrier.h>


#define FFT128
#include <fft.hpp>

// Cache warming function
#ifdef WARM_CACHE
#define CACHE_LINE_COMPLEX 8
__attribute__ ((noinline))
static int warmup(FP32Complex *in, FP32Complex *out, float bsg_attr_remote * bsg_attr_noalias tw, int N, int num_iter)
{
  for (int i = __bsg_id*CACHE_LINE_COMPLEX; i < N*num_iter; i += bsg_tiles_X*bsg_tiles_Y*CACHE_LINE_COMPLEX) {
      asm volatile ("lw x0, %[p]" :: [p] "m" (in[i]));
      asm volatile ("lw x0, %[p]" :: [p] "m" (out[i]));
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
kernel_fft(FP32Complex *in, FP32Complex *out, float bsg_attr_remote * bsg_attr_noalias tw, int N, int num_iter) {


    bsg_barrier_hw_tile_group_init();
    bsg_fence();

    #ifdef WARM_CACHE
    warmup(in, out, tw, N, num_iter);
    #endif

    // Kernel Start
    bsg_cuda_print_stat_kernel_start();
    bsg_fence();
  
    int stat_count = 0;
    
    for (int i = 0; i < num_iter; i++) {
      // step 1
      //bsg_cuda_print_stat_start(stat_count);
      load_fft_scale_no_twiddle(&in[N*i], fft_workset, tw, __bsg_id, 128, 128, N);
      //bsg_cuda_print_stat_end(stat_count); stat_count++;
      asm volatile("": : :"memory");
      bsg_barrier_hw_tile_group_sync();

      // step 2
      //bsg_cuda_print_stat_start(stat_count);
      tg_mem_square_transpose_inp(fft_workset, 128);
      //bsg_cuda_print_stat_end(stat_count); stat_count++;
      asm volatile("": : :"memory");
      bsg_barrier_hw_tile_group_sync();

      // step 3
      //bsg_cuda_print_stat_start(stat_count);
      fft_store(fft_workset, &out[N*i], __bsg_id, 128, 128, N);
      bsg_fence();
      asm volatile("": : :"memory");
      bsg_barrier_hw_tile_group_sync();
      //bsg_cuda_print_stat_end(stat_count); stat_count++;
    }


    // Kernel end
    bsg_cuda_print_stat_kernel_end();
    bsg_fence();
    asm volatile("": : :"memory");
    bsg_barrier_hw_tile_group_sync();

    return 0;
}
