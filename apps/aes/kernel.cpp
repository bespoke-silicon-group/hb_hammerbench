#include <bsg_manycore.h>
#include <bsg_cuda_lite_barrier.h>
#include "bsg_barrier_multipod.h"
#include "aes_kernel.hpp"

// Multipod barrier;
//volatile int done[NUM_POD_X]={0};
//int alert = 0;

// WARM cache;
#define CACHE_LINE_IN_BYTES 64
__attribute__ ((noinline))
void warmup(struct AES_ctx *ctx, uint8_t* buf, size_t length, int niters) {
  // prefetch ctx
  for (int n = 0; n < niters; n++) {
    struct AES_ctx *curr_ctx = &ctx[(__bsg_id*niters) + n];
    asm volatile ("lw x0, %[p]" :: [p] "m" (curr_ctx->RoundKey[0]));
    asm volatile ("lw x0, %[p]" :: [p] "m" (curr_ctx->RoundKey[CACHE_LINE_IN_BYTES]));
    asm volatile ("lw x0, %[p]" :: [p] "m" (curr_ctx->RoundKey[CACHE_LINE_IN_BYTES*2]));
    asm volatile ("lw x0, %[p]" :: [p] "m" (curr_ctx->Iv));
  }

  // prefetch buf;
  uint8_t *my_buf = &buf[(__bsg_id*niters*length)];
  for (int i = 0; i < length*niters; i += CACHE_LINE_IN_BYTES) {
    asm volatile ("lw x0, %[p]" :: [p] "m" (my_buf[i]));
  }
}



// Kernel main;
extern "C"
int kernel(struct AES_ctx *ctx, uint8_t* buf, size_t length, int niters, int pod_id)
{
  bsg_barrier_tile_group_init();
  bsg_barrier_tile_group_sync();
  //warmup(ctx, buf, length, niters);
  bsg_fence();

  // Kernel start;
  bsg_barrier_tile_group_sync();
  //bsg_barrier_multipod(pod_id, NUM_POD_X, done, &alert);
  //bsg_cuda_print_stat_kernel_start();

  //// calculate global_x,y;
  //int cfg_pod;
  //asm volatile ("csrr %[cfg_pod], 0x360" : [cfg_pod] "=r" (cfg_pod));
  //int pod_x = cfg_pod & 0x7;
  //int pod_y = (cfg_pod & 0x78) >> 3; // 1 = podrow 0, 3 = podrow1;
  //int global_x = (pod_x<<4) + __bsg_x;
  //int global_y = (pod_y<<3) + __bsg_y;
  //
  //bsg_printf("Pod coord: %d, %d\n", pod_x, pod_y);

  for (int i = 0; i < niters; i++) {
    AES_CBC_encrypt_buffer(
      &ctx[(__bsg_id*niters)+i],
      &buf[(__bsg_id*niters*length) + (length*i)],
      length);
  }


  // kernel end;
  //bsg_fence();
  //bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_tile_group_sync();
  return 0;
}
