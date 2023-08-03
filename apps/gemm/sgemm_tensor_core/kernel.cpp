#include <bsg_manycore.h>
#include <bsg_cuda_lite_barrier.h>
#include <cstring>
#include <cstdint>
#include <math.h>

#define BLOCK_DIM 16
#define NUM_BLOCK (Z/(BLOCK_DIM*2))
#define SUB_DIM 4


float block1[BLOCK_DIM*BLOCK_DIM];
float block2[BLOCK_DIM*BLOCK_DIM];
float block_out[BLOCK_DIM*BLOCK_DIM];


// Reset out
inline void reset_out() {
  bsg_unroll(1)
  for (int y = 0; y < BLOCK_DIM; y++) {
    bsg_unroll(BLOCK_DIM)
    for (int x = 0; x < BLOCK_DIM; x++) {
      block_out[(BLOCK_DIM*y)+x] = 0.0f;
    }
  }
}




// Load input block from DRAM
inline void load_block( float* dst,
                        float bsg_attr_remote * bsg_attr_noalias src)
{
  bsg_unroll(1)
  for (int y = 0; y < BLOCK_DIM; y+=1) {
    register float tmp00 =  src[(N*y)+0];
    register float tmp01 =  src[(N*y)+1];
    register float tmp02 =  src[(N*y)+2];
    register float tmp03 =  src[(N*y)+3];
    register float tmp04 =  src[(N*y)+4];
    register float tmp05 =  src[(N*y)+5];
    register float tmp06 =  src[(N*y)+6];
    register float tmp07 =  src[(N*y)+7];
    register float tmp08 =  src[(N*y)+8];
    register float tmp09 =  src[(N*y)+9];
    register float tmp10 =  src[(N*y)+10];
    register float tmp11 =  src[(N*y)+11];
    register float tmp12 =  src[(N*y)+12];
    register float tmp13 =  src[(N*y)+13];
    register float tmp14 =  src[(N*y)+14];
    register float tmp15 =  src[(N*y)+15];
    asm volatile ("" ::: "memory");
    dst[(BLOCK_DIM*y)+0] = tmp00;
    dst[(BLOCK_DIM*y)+1] = tmp01;
    dst[(BLOCK_DIM*y)+2] = tmp02;
    dst[(BLOCK_DIM*y)+3] = tmp03;
    dst[(BLOCK_DIM*y)+4] = tmp04;
    dst[(BLOCK_DIM*y)+5] = tmp05;
    dst[(BLOCK_DIM*y)+6] = tmp06;
    dst[(BLOCK_DIM*y)+7] = tmp07;
    dst[(BLOCK_DIM*y)+8] = tmp08;
    dst[(BLOCK_DIM*y)+9] = tmp09;
    dst[(BLOCK_DIM*y)+10] = tmp10;
    dst[(BLOCK_DIM*y)+11] = tmp11;
    dst[(BLOCK_DIM*y)+12] = tmp12;
    dst[(BLOCK_DIM*y)+13] = tmp13;
    dst[(BLOCK_DIM*y)+14] = tmp14;
    dst[(BLOCK_DIM*y)+15] = tmp15;
  }
}




// Store output to DRAM
inline void store_block(float* dst) {
  bsg_unroll(1)
  for (int y = 0; y < BLOCK_DIM; y+=1) {
    register float tp00 =  block_out[(BLOCK_DIM*y)+0];
    register float tp01 =  block_out[(BLOCK_DIM*y)+1];
    register float tp02 =  block_out[(BLOCK_DIM*y)+2];
    register float tp03 =  block_out[(BLOCK_DIM*y)+3];
    register float tp04 =  block_out[(BLOCK_DIM*y)+4];
    register float tp05 =  block_out[(BLOCK_DIM*y)+5];
    register float tp06 =  block_out[(BLOCK_DIM*y)+6];
    register float tp07 =  block_out[(BLOCK_DIM*y)+7];
    register float tp08 =  block_out[(BLOCK_DIM*y)+8];
    register float tp09 =  block_out[(BLOCK_DIM*y)+9];
    register float tp10 =  block_out[(BLOCK_DIM*y)+10];
    register float tp11 =  block_out[(BLOCK_DIM*y)+11];
    register float tp12 =  block_out[(BLOCK_DIM*y)+12];
    register float tp13 =  block_out[(BLOCK_DIM*y)+13];
    register float tp14 =  block_out[(BLOCK_DIM*y)+14];
    register float tp15 =  block_out[(BLOCK_DIM*y)+15];
    asm volatile ("" ::: "memory");
    dst[(N*y)+0] = tp00; 
    dst[(N*y)+1] = tp01; 
    dst[(N*y)+2] = tp02; 
    dst[(N*y)+3] = tp03; 
    dst[(N*y)+4] = tp04; 
    dst[(N*y)+5] = tp05; 
    dst[(N*y)+6] = tp06; 
    dst[(N*y)+7] = tp07; 
    dst[(N*y)+8] = tp08; 
    dst[(N*y)+9] = tp09; 
    dst[(N*y)+10] = tp10; 
    dst[(N*y)+11] = tp11; 
    dst[(N*y)+12] = tp12; 
    dst[(N*y)+13] = tp13; 
    dst[(N*y)+14] = tp14; 
    dst[(N*y)+15] = tp15; 
  }
}




static inline void compute_block() {
  bsg_unroll(1)
  for (int y = 0; y < N; y += SUB_DIM) {
    float *sub_block_out = &block_out[N*y]; 
    // load accum
    asm volatile ("flw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("flw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("flw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("flw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("flw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("flw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("flw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("flw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("flw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("flw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("flw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("flw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("flw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("flw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("flw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("flw f0, %[p]" :: [p] "m" (sub_block_out[0]));
  
    bsg_unroll(1)
    for (int z = 0; z < (BLOCK_DIM*2/SUB_DIM); z+=SUB_DIM) {
      asm volatile ("nop"); // LD8.f16 A00[0:1]
      asm volatile ("nop"); // LD8.f16 A00[2:3]
      asm volatile ("nop"); // mmul4.f16 A00, accum0
      asm volatile ("nop"); // mmul4.f16 A00, accum0
      asm volatile ("nop"); // mmul4.f16 A00, accum1
      asm volatile ("nop"); // mmul4.f16 A00, accum1
    }

    // store accum
    asm volatile ("fsw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("fsw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("fsw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("fsw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("fsw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("fsw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("fsw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("fsw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("fsw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("fsw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("fsw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("fsw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("fsw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("fsw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("fsw f0, %[p]" :: [p] "m" (sub_block_out[0]));
    asm volatile ("fsw f0, %[p]" :: [p] "m" (sub_block_out[0]));
  }
}




extern "C"
int kernel_mm_opt(float *mat1, float *mat2, float *result)
{
  bsg_barrier_hw_tile_group_init();


  // start kernel
  bsg_barrier_hw_tile_group_sync();
  bsg_cuda_print_stat_kernel_start();


  // reset output;
  bsg_unroll(1)
  for (int by = __bsg_y*BLOCK_DIM; by < N; by += (bsg_tiles_Y*BLOCK_DIM)) {
    bsg_unroll(1)
    for (int bx = __bsg_x*BLOCK_DIM; bx < N; bx += (bsg_tiles_X*BLOCK_DIM)) {

      // reset out
      reset_out();

      // Iterate
      int curr_id = __bsg_x % NUM_BLOCK;
      bsg_unroll(1)
      for (int z = 0; z < NUM_BLOCK; z++) {
        // load mat1, mat2 block
        float *src1 = &mat1[(by*Z/2)+(BLOCK_DIM*curr_id)];
        float *src2 = &mat2[(bx*Z/2)+(BLOCK_DIM*curr_id)];
        load_block(block1, src1);
        load_block(block2, src2);

        // compute block output
        compute_block();

        // next block id
        curr_id = (curr_id+1) % NUM_BLOCK;
      } 
    
      // store block
      float *dst = &result[(N*by)+bx];
      store_block(dst);
    }
  }

  // end kernel
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();
  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();
  return 0;
}
