#include <bsg_manycore.h>
#include <bsg_cuda_lite_barrier.h>
#include <cstring>
#include <cstdint>
#include <math.h>

//#define BLOCK_DIM_X 8
//#define BLOCK_DIM_Y 16
#define SUB_DIM 4
#define STRIDE (BLOCK_DIM_X*BATCH)

float block1[BLOCK_DIM_X*BLOCK_DIM_Y];
float block2[BLOCK_DIM_X*BLOCK_DIM_Y];
float block_out[BLOCK_DIM_Y*BLOCK_DIM_Y];


// load and reset
inline void load_and_reset(float *mat1, float *mat2) {
  int * curr_block_out_row = (int *) block_out;
  bsg_unroll(1)
  for (int y = 0; y < BLOCK_DIM_Y; y++) {
    register float a0 = mat1[(STRIDE*y)+0];
    register float a1 = mat1[(STRIDE*y)+1];
    register float a2 = mat1[(STRIDE*y)+2];
    register float a3 = mat1[(STRIDE*y)+3];

    register float b0 = mat2[(STRIDE*y)+0];
    register float b1 = mat2[(STRIDE*y)+1];
    register float b2 = mat2[(STRIDE*y)+2];
    register float b3 = mat2[(STRIDE*y)+3];

    register float a4 = mat1[(STRIDE*y)+4];
    register float a5 = mat1[(STRIDE*y)+5];
    register float a6 = mat1[(STRIDE*y)+6];
    register float a7 = mat1[(STRIDE*y)+7];

    register float b4 = mat2[(STRIDE*y)+4];
    register float b5 = mat2[(STRIDE*y)+5];
    register float b6 = mat2[(STRIDE*y)+6];
    register float b7 = mat2[(STRIDE*y)+7];
    asm volatile ("" ::: "memory");
    
    curr_block_out_row[(2*BLOCK_DIM_X*y)+0] = 0;
    curr_block_out_row[(2*BLOCK_DIM_X*y)+1] = 0;
    curr_block_out_row[(2*BLOCK_DIM_X*y)+2] = 0;
    curr_block_out_row[(2*BLOCK_DIM_X*y)+3] = 0;
    curr_block_out_row[(2*BLOCK_DIM_X*y)+4] = 0;
    curr_block_out_row[(2*BLOCK_DIM_X*y)+5] = 0;
    curr_block_out_row[(2*BLOCK_DIM_X*y)+6] = 0;
    curr_block_out_row[(2*BLOCK_DIM_X*y)+7] = 0;
    curr_block_out_row[(2*BLOCK_DIM_X*y)+8] = 0;
    curr_block_out_row[(2*BLOCK_DIM_X*y)+9] = 0;
    curr_block_out_row[(2*BLOCK_DIM_X*y)+10] = 0;
    curr_block_out_row[(2*BLOCK_DIM_X*y)+11] = 0;
    curr_block_out_row[(2*BLOCK_DIM_X*y)+12] = 0;
    curr_block_out_row[(2*BLOCK_DIM_X*y)+13] = 0;
    curr_block_out_row[(2*BLOCK_DIM_X*y)+14] = 0;
    curr_block_out_row[(2*BLOCK_DIM_X*y)+15] = 0;
    asm volatile ("" ::: "memory");

    block1[(BLOCK_DIM_X*y)+0] = a0;
    block1[(BLOCK_DIM_X*y)+1] = a1;
    block1[(BLOCK_DIM_X*y)+2] = a2;
    block1[(BLOCK_DIM_X*y)+3] = a3;
    block2[(BLOCK_DIM_X*y)+0] = b0;
    block2[(BLOCK_DIM_X*y)+1] = b1;
    block2[(BLOCK_DIM_X*y)+2] = b2;
    block2[(BLOCK_DIM_X*y)+3] = b3;
    block1[(BLOCK_DIM_X*y)+4] = a4;
    block1[(BLOCK_DIM_X*y)+5] = a5;
    block1[(BLOCK_DIM_X*y)+6] = a6;
    block1[(BLOCK_DIM_X*y)+7] = a7;
    block2[(BLOCK_DIM_X*y)+4] = b4;
    block2[(BLOCK_DIM_X*y)+5] = b5;
    block2[(BLOCK_DIM_X*y)+6] = b6;
    block2[(BLOCK_DIM_X*y)+7] = b7;
    asm volatile ("" ::: "memory");
  }
}

// Store output to DRAM
inline void store_block(float* dst) {
  bsg_unroll(1)
  for (int y = 0; y < BLOCK_DIM_Y; y+=2) {
    register float tp00 =  block_out[(BLOCK_DIM_X*2*y)+0];
    register float tp01 =  block_out[(BLOCK_DIM_X*2*y)+1];
    register float tp02 =  block_out[(BLOCK_DIM_X*2*y)+2];
    register float tp03 =  block_out[(BLOCK_DIM_X*2*y)+3];
    register float tp04 =  block_out[(BLOCK_DIM_X*2*y)+4];
    register float tp05 =  block_out[(BLOCK_DIM_X*2*y)+5];
    register float tp06 =  block_out[(BLOCK_DIM_X*2*y)+6];
    register float tp07 =  block_out[(BLOCK_DIM_X*2*y)+7];
    register float tp08 =  block_out[(BLOCK_DIM_X*2*y)+8];
    register float tp09 =  block_out[(BLOCK_DIM_X*2*y)+9];
    register float tp10 =  block_out[(BLOCK_DIM_X*2*y)+10];
    register float tp11 =  block_out[(BLOCK_DIM_X*2*y)+11];
    register float tp12 =  block_out[(BLOCK_DIM_X*2*y)+12];
    register float tp13 =  block_out[(BLOCK_DIM_X*2*y)+13];
    register float tp14 =  block_out[(BLOCK_DIM_X*2*y)+14];
    register float tp15 =  block_out[(BLOCK_DIM_X*2*y)+15];

    register float tp16 =  block_out[(BLOCK_DIM_X*2*(y+1))+0];
    register float tp17 =  block_out[(BLOCK_DIM_X*2*(y+1))+1];
    register float tp18 =  block_out[(BLOCK_DIM_X*2*(y+1))+2];
    register float tp19 =  block_out[(BLOCK_DIM_X*2*(y+1))+3];
    register float tp20 =  block_out[(BLOCK_DIM_X*2*(y+1))+4];
    register float tp21 =  block_out[(BLOCK_DIM_X*2*(y+1))+5];
    register float tp22 =  block_out[(BLOCK_DIM_X*2*(y+1))+6];
    register float tp23 =  block_out[(BLOCK_DIM_X*2*(y+1))+7];
    register float tp24 =  block_out[(BLOCK_DIM_X*2*(y+1))+8];
    register float tp25 =  block_out[(BLOCK_DIM_X*2*(y+1))+9];
    register float tp26 =  block_out[(BLOCK_DIM_X*2*(y+1))+10];
    register float tp27 =  block_out[(BLOCK_DIM_X*2*(y+1))+11];
    register float tp28 =  block_out[(BLOCK_DIM_X*2*(y+1))+12];
    register float tp29 =  block_out[(BLOCK_DIM_X*2*(y+1))+13];
    register float tp30 =  block_out[(BLOCK_DIM_X*2*(y+1))+14];
    register float tp31 =  block_out[(BLOCK_DIM_X*2*(y+1))+15];
    asm volatile ("" ::: "memory");
    dst[(STRIDE*y)+0] = tp00; 
    dst[(STRIDE*y)+1] = tp01; 
    dst[(STRIDE*y)+2] = tp02; 
    dst[(STRIDE*y)+3] = tp03; 
    dst[(STRIDE*y)+4] = tp04; 
    dst[(STRIDE*y)+5] = tp05; 
    dst[(STRIDE*y)+6] = tp06; 
    dst[(STRIDE*y)+7] = tp07; 
    dst[(STRIDE*y)+8] = tp08; 
    dst[(STRIDE*y)+9] = tp09; 
    dst[(STRIDE*y)+10] = tp10; 
    dst[(STRIDE*y)+11] = tp11; 
    dst[(STRIDE*y)+12] = tp12; 
    dst[(STRIDE*y)+13] = tp13; 
    dst[(STRIDE*y)+14] = tp14; 
    dst[(STRIDE*y)+15] = tp15; 

    dst[(STRIDE*(y+1))+0] = tp16; 
    dst[(STRIDE*(y+1))+1] = tp17; 
    dst[(STRIDE*(y+1))+2] = tp18; 
    dst[(STRIDE*(y+1))+3] = tp19; 
    dst[(STRIDE*(y+1))+4] = tp20; 
    dst[(STRIDE*(y+1))+5] = tp21; 
    dst[(STRIDE*(y+1))+6] = tp22; 
    dst[(STRIDE*(y+1))+7] = tp23; 
    dst[(STRIDE*(y+1))+8] = tp24; 
    dst[(STRIDE*(y+1))+9] = tp25; 
    dst[(STRIDE*(y+1))+10] = tp26; 
    dst[(STRIDE*(y+1))+11] = tp27; 
    dst[(STRIDE*(y+1))+12] = tp28; 
    dst[(STRIDE*(y+1))+13] = tp29; 
    dst[(STRIDE*(y+1))+14] = tp30; 
    dst[(STRIDE*(y+1))+15] = tp31; 
  }
}


static inline void compute_block() {
  bsg_unroll(1)
  for (int sy = 0; sy < BLOCK_DIM_Y; sy+=SUB_DIM) {
    float *sub_block_out = &block_out[BLOCK_DIM_Y*sy];
    // load accum for 16x4
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
  
    asm volatile ("nop"); // LD8.f16 A00[0:1]
    asm volatile ("nop"); // LD8.f16 A00[2:3]
    asm volatile ("nop"); // mmul4.f16 A00, accum0
    asm volatile ("nop"); // mmul4.f16 A00, accum0
    asm volatile ("nop"); // mmul4.f16 A00, accum1
    asm volatile ("nop"); // mmul4.f16 A00, accum1
    asm volatile ("nop"); // mmul4.f16 A00, accum2
    asm volatile ("nop"); // mmul4.f16 A00, accum2
    asm volatile ("nop"); // mmul4.f16 A00, accum3
    asm volatile ("nop"); // mmul4.f16 A00, accum3

    asm volatile ("nop"); // LD8.f16 A00[0:1]
    asm volatile ("nop"); // LD8.f16 A00[2:3]
    asm volatile ("nop"); // mmul4.f16 A00, accum0
    asm volatile ("nop"); // mmul4.f16 A00, accum0
    asm volatile ("nop"); // mmul4.f16 A00, accum1
    asm volatile ("nop"); // mmul4.f16 A00, accum1
    asm volatile ("nop"); // mmul4.f16 A00, accum2
    asm volatile ("nop"); // mmul4.f16 A00, accum2
    asm volatile ("nop"); // mmul4.f16 A00, accum3
    asm volatile ("nop"); // mmul4.f16 A00, accum3

    asm volatile ("nop"); // LD8.f16 A00[0:1]
    asm volatile ("nop"); // LD8.f16 A00[2:3]
    asm volatile ("nop"); // mmul4.f16 A00, accum0
    asm volatile ("nop"); // mmul4.f16 A00, accum0
    asm volatile ("nop"); // mmul4.f16 A00, accum1
    asm volatile ("nop"); // mmul4.f16 A00, accum1
    asm volatile ("nop"); // mmul4.f16 A00, accum2
    asm volatile ("nop"); // mmul4.f16 A00, accum2
    asm volatile ("nop"); // mmul4.f16 A00, accum3
    asm volatile ("nop"); // mmul4.f16 A00, accum3

    asm volatile ("nop"); // LD8.f16 A00[0:1]
    asm volatile ("nop"); // LD8.f16 A00[2:3]
    asm volatile ("nop"); // mmul4.f16 A00, accum0
    asm volatile ("nop"); // mmul4.f16 A00, accum0
    asm volatile ("nop"); // mmul4.f16 A00, accum1
    asm volatile ("nop"); // mmul4.f16 A00, accum1
    asm volatile ("nop"); // mmul4.f16 A00, accum2
    asm volatile ("nop"); // mmul4.f16 A00, accum2
    asm volatile ("nop"); // mmul4.f16 A00, accum3
    asm volatile ("nop"); // mmul4.f16 A00, accum3

    // store accum for 16x4
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

#ifdef WARM_CACHE
#define CACHE_LINE_WORDS 16
  for (int i = __bsg_id*CACHE_LINE_WORDS; i < BLOCK_DIM_X*BLOCK_DIM_Y*BATCH; i+=(CACHE_LINE_WORDS*bsg_tiles_X*bsg_tiles_Y)) {
    asm volatile ("lw x0, %[p]" :: [p] "m" (mat1[i]));
    asm volatile ("lw x0, %[p]" :: [p] "m" (mat2[i]));
  }
  for (int i = __bsg_id*CACHE_LINE_WORDS; i < BLOCK_DIM_X*BLOCK_DIM_Y*BATCH*2; i+=(CACHE_LINE_WORDS*bsg_tiles_X*bsg_tiles_Y)) {
    asm volatile ("sw x0, %[p]" :: [p] "m" (result[i]));
  }
  bsg_fence();
#endif

  // calculate logical pod id;
  //uint32_t __logical_dim_x = bsg_tiles_X * UNFOLD;
  //uint32_t __logical_dim_y = bsg_tiles_Y / UNFOLD;
  //uint32_t __logical_bsg_x = __bsg_x + (__bsg_y/__logical_dim_y)*bsg_tiles_X;
  //uint32_t __logical_bsg_y = __bsg_y % __logical_dim_y;

  // start kernel
  bsg_barrier_hw_tile_group_sync();
  bsg_cuda_print_stat_kernel_start();

  bsg_unroll(1)
  for (int i = __bsg_id; i < BATCH; i += (bsg_tiles_X*bsg_tiles_Y)) {
    load_and_reset(&mat1[(BLOCK_DIM_X)*i], &mat2[(BLOCK_DIM_X)*i]);
    compute_block();
    store_block(&result[(BLOCK_DIM_X*2)*i]);
  } 

  // end kernel
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();
  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();
  return 0;
}
