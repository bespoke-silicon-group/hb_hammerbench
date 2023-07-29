#include <bsg_manycore.h>
#include <bsg_cuda_lite_barrier.h>
#include <cstring>
#include <cstdint>
#include <math.h>

#define BLOCK_DIM 16
#define NUM_BLOCK (N/BLOCK_DIM)
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
  for (int y = 0; y < BLOCK_DIM; y+=2) {
    register float tmp00 =  src[(N*y)+0];
    bsg_fence();
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

    register float tmp16 =  src[(N*(y+1))+0];
    bsg_fence();
    register float tmp17 =  src[(N*(y+1))+1];
    register float tmp18 =  src[(N*(y+1))+2];
    register float tmp19 =  src[(N*(y+1))+3];
    register float tmp20 =  src[(N*(y+1))+4];
    register float tmp21 =  src[(N*(y+1))+5];
    register float tmp22 =  src[(N*(y+1))+6];
    register float tmp23 =  src[(N*(y+1))+7];
    register float tmp24 =  src[(N*(y+1))+8];
    register float tmp25 =  src[(N*(y+1))+9];
    register float tmp26 =  src[(N*(y+1))+10];
    register float tmp27 =  src[(N*(y+1))+11];
    register float tmp28 =  src[(N*(y+1))+12];
    register float tmp29 =  src[(N*(y+1))+13];
    register float tmp30 =  src[(N*(y+1))+14];
    register float tmp31 =  src[(N*(y+1))+15];
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

    dst[(BLOCK_DIM*(y+1))+0] = tmp16;
    dst[(BLOCK_DIM*(y+1))+1] = tmp17;
    dst[(BLOCK_DIM*(y+1))+2] = tmp18;
    dst[(BLOCK_DIM*(y+1))+3] = tmp19;
    dst[(BLOCK_DIM*(y+1))+4] = tmp20;
    dst[(BLOCK_DIM*(y+1))+5] = tmp21;
    dst[(BLOCK_DIM*(y+1))+6] = tmp22;
    dst[(BLOCK_DIM*(y+1))+7] = tmp23;
    dst[(BLOCK_DIM*(y+1))+8] = tmp24;
    dst[(BLOCK_DIM*(y+1))+9] = tmp25;
    dst[(BLOCK_DIM*(y+1))+10] = tmp26;
    dst[(BLOCK_DIM*(y+1))+11] = tmp27;
    dst[(BLOCK_DIM*(y+1))+12] = tmp28;
    dst[(BLOCK_DIM*(y+1))+13] = tmp29;
    dst[(BLOCK_DIM*(y+1))+14] = tmp30;
    dst[(BLOCK_DIM*(y+1))+15] = tmp31;
  }
}


// Store output to DRAM
inline void store_block(float* dst) {
  bsg_unroll(1)
  for (int y = 0; y < BLOCK_DIM; y+=2) {
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

    register float tp16 =  block_out[(BLOCK_DIM*(y+1))+0];
    register float tp17 =  block_out[(BLOCK_DIM*(y+1))+1];
    register float tp18 =  block_out[(BLOCK_DIM*(y+1))+2];
    register float tp19 =  block_out[(BLOCK_DIM*(y+1))+3];
    register float tp20 =  block_out[(BLOCK_DIM*(y+1))+4];
    register float tp21 =  block_out[(BLOCK_DIM*(y+1))+5];
    register float tp22 =  block_out[(BLOCK_DIM*(y+1))+6];
    register float tp23 =  block_out[(BLOCK_DIM*(y+1))+7];
    register float tp24 =  block_out[(BLOCK_DIM*(y+1))+8];
    register float tp25 =  block_out[(BLOCK_DIM*(y+1))+9];
    register float tp26 =  block_out[(BLOCK_DIM*(y+1))+10];
    register float tp27 =  block_out[(BLOCK_DIM*(y+1))+11];
    register float tp28 =  block_out[(BLOCK_DIM*(y+1))+12];
    register float tp29 =  block_out[(BLOCK_DIM*(y+1))+13];
    register float tp30 =  block_out[(BLOCK_DIM*(y+1))+14];
    register float tp31 =  block_out[(BLOCK_DIM*(y+1))+15];
    asm volatile ("" ::: "memory");
    dst[(N*y)+0] = tp00; 
    bsg_fence();
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

    dst[(N*(y+1))+0] = tp16; 
    bsg_fence();
    dst[(N*(y+1))+1] = tp17; 
    dst[(N*(y+1))+2] = tp18; 
    dst[(N*(y+1))+3] = tp19; 
    dst[(N*(y+1))+4] = tp20; 
    dst[(N*(y+1))+5] = tp21; 
    dst[(N*(y+1))+6] = tp22; 
    dst[(N*(y+1))+7] = tp23; 
    dst[(N*(y+1))+8] = tp24; 
    dst[(N*(y+1))+9] = tp25; 
    dst[(N*(y+1))+10] = tp26; 
    dst[(N*(y+1))+11] = tp27; 
    dst[(N*(y+1))+12] = tp28; 
    dst[(N*(y+1))+13] = tp29; 
    dst[(N*(y+1))+14] = tp30; 
    dst[(N*(y+1))+15] = tp31; 
  }
}

static inline void compute_block() {
  bsg_unroll(1)
  for (int sy = 0; sy < BLOCK_DIM; sy += SUB_DIM) {
    // sub block_out
    float *sub_block_out = &block_out[(BLOCK_DIM*sy)];
    // LD4.f32 (load accum 16x4)
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
    for (int sz = 0; sz < BLOCK_DIM*2; sz += SUB_DIM) {
      // Load mat A (4x4) (LD4.f16)
      asm volatile ("flw f0, %[p]" :: [p] "m" (block1[0]));
      asm volatile ("flw f0, %[p]" :: [p] "m" (block1[0]));
      asm volatile ("flw f0, %[p]" :: [p] "m" (block1[0]));
      asm volatile ("flw f0, %[p]" :: [p] "m" (block1[0]));
      // Load mat B (16x4) (LD4.f16) + mmul4.f16
      asm volatile ("fmadd.s f1, f2, f3, f4");
      asm volatile ("fmadd.s f1, f2, f3, f4");
      asm volatile ("fmadd.s f1, f2, f3, f4");
      asm volatile ("fmadd.s f1, f2, f3, f4");
      asm volatile ("fmadd.s f1, f2, f3, f4");
      asm volatile ("fmadd.s f1, f2, f3, f4");
      asm volatile ("fmadd.s f1, f2, f3, f4");
      asm volatile ("fmadd.s f1, f2, f3, f4");
      asm volatile ("fmadd.s f1, f2, f3, f4");
      asm volatile ("fmadd.s f1, f2, f3, f4");
      asm volatile ("fmadd.s f1, f2, f3, f4");
      asm volatile ("fmadd.s f1, f2, f3, f4");
      asm volatile ("fmadd.s f1, f2, f3, f4");
      asm volatile ("fmadd.s f1, f2, f3, f4");
      asm volatile ("fmadd.s f1, f2, f3, f4");
      asm volatile ("fmadd.s f1, f2, f3, f4");
    }
    // ST4.f32 (store accum 16x4)
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
  for (int i = __bsg_id*CACHE_LINE_WORDS; i < N*N; i+=(CACHE_LINE_WORDS*bsg_tiles_X*bsg_tiles_Y)) {
    asm volatile ("lw x0, %[p]" :: [p] "m" (mat1[i]));
    asm volatile ("lw x0, %[p]" :: [p] "m" (mat2[i]));
    asm volatile ("sw x0, %[p]" :: [p] "m" (result[i]));
  }
  bsg_fence();
#endif

  // calculate logical pod id;
  uint32_t __logical_dim_x = bsg_tiles_X * UNFOLD;
  uint32_t __logical_dim_y = bsg_tiles_Y / UNFOLD;
  uint32_t __logical_bsg_x = __bsg_x + (__bsg_y/__logical_dim_y)*bsg_tiles_X;
  uint32_t __logical_bsg_y = __bsg_y % __logical_dim_y;

  // start kernel
  bsg_barrier_hw_tile_group_sync();
  bsg_cuda_print_stat_kernel_start();


  // reset output;
  bsg_unroll(1)
  for (int by = __logical_bsg_y; by < NUM_BLOCK; by += __logical_dim_y) {
    bsg_unroll(1)
    for (int bx = __logical_bsg_x; bx < NUM_BLOCK; bx += __logical_dim_x) {

      // reset out
      reset_out();

      // starting block id
      int curr_id = __logical_bsg_x % NUM_BLOCK;

      // Iterate
      for (int z = 0; z < NUM_BLOCK; z++) {
        // load mat1, mat2 block
        float *src1 = &mat1[(N*BLOCK_DIM*by)+(BLOCK_DIM*curr_id)];
        float *src2 = &mat2[(N*BLOCK_DIM*curr_id)+(BLOCK_DIM*bx)];
        load_block(block1, src1);
        load_block(block2, src2);

        // compute block output
        compute_block();

        // next block id
        curr_id = (curr_id+1) % NUM_BLOCK;
      } 
    
      // store block
      float *dst = &result[(N*BLOCK_DIM*by)+(BLOCK_DIM*bx)];
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
