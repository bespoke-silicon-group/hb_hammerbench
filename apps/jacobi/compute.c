#include <bsg_manycore.h>
#include "bsg_cuda_lite_barrier.h"
#include <stdbool.h>
//#define Index3D(nx,ny,nz,x,y,z) ((((y*nx)+x)*nz)+z)
#define Index3D(nx,ny,nz,x,y,z) (((((y)*nx)+(x))*nz)+(z))

__attribute__ ((always_inline)) 
void copySelf(float bsg_attr_remote * bsg_attr_noalias src, float bsg_attr_remote * bsg_attr_noalias dst, int start_idx) {
  // set first
  dst[0] = src[start_idx-1];

  // set middle
  float bsg_attr_remote * bsg_attr_noalias temp_dst = &dst[1];
  float bsg_attr_remote * bsg_attr_noalias temp_src = &src[start_idx];

  bsg_unroll(1)
  for (int i = 0; i < (LOCAL_SIZE/16); i++) {
    register float tmp00 =  temp_src[0];
    register float tmp01 =  temp_src[1];
    register float tmp02 =  temp_src[2];
    register float tmp03 =  temp_src[3];
    register float tmp04 =  temp_src[4];
    register float tmp05 =  temp_src[5];
    register float tmp06 =  temp_src[6];
    register float tmp07 =  temp_src[7];
    register float tmp08 =  temp_src[8];
    register float tmp09 =  temp_src[9];
    register float tmp10 =  temp_src[10];
    register float tmp11 =  temp_src[11];
    register float tmp12 =  temp_src[12];
    register float tmp13 =  temp_src[13];
    register float tmp14 =  temp_src[14];
    register float tmp15 =  temp_src[15];
    asm volatile("": : :"memory");
    temp_dst[0] = tmp00;
    temp_dst[1] = tmp01;
    temp_dst[2] = tmp02;
    temp_dst[3] = tmp03;
    temp_dst[4] = tmp04;
    temp_dst[5] = tmp05;
    temp_dst[6] = tmp06;
    temp_dst[7] = tmp07;
    temp_dst[8] = tmp08;
    temp_dst[9] = tmp09;
    temp_dst[10] = tmp10;
    temp_dst[11] = tmp11;
    temp_dst[12] = tmp12;
    temp_dst[13] = tmp13;
    temp_dst[14] = tmp14;
    temp_dst[15] = tmp15;
    temp_dst += 16;
    temp_src += 16;
  }

  // set last
  temp_dst[0] = temp_src[0];
}

void compute (
  int c0, int c1,
  float bsg_attr_remote * bsg_attr_noalias A0,
  float bsg_attr_remote * bsg_attr_noalias Anext,
  float bsg_attr_remote * bsg_attr_noalias a_left,
  float bsg_attr_remote * bsg_attr_noalias a_right,
  float bsg_attr_remote * bsg_attr_noalias a_up,
  float bsg_attr_remote * bsg_attr_noalias a_down,
  float bsg_attr_remote * bsg_attr_noalias a_self,
  const int nx, const int ny, const int nz
) {

  float c0f = (float) c0;
  float c1f = (float) c1;
  for (int ii = 0; ii < nz-2; ii += LOCAL_SIZE) {

    copySelf(&A0[Index3D(nx,ny,nz,__bsg_x,__bsg_y,0)], a_self, ii+1);
    bsg_barrier_hw_tile_group_sync();

    // compute 4 at a time
    bsg_unroll(1)
    for (int i = 0; i < LOCAL_SIZE; i += 4) {
      int self_idx = i+1;
      register float left0 = a_left[i];
      register float right0 = a_right[i];
      register float up0 = a_up[i];
      register float down0 = a_down[i];

      register float left1 = a_left[i+1];
      register float right1 = a_right[i+1];
      register float up1 = a_up[i+1];
      register float down1 = a_down[i+1];

      register float left2 = a_left[i+2];
      register float right2 = a_right[i+2];
      register float up2 = a_up[i+2];
      register float down2 = a_down[i+2];

      register float left3 = a_left[i+3];
      register float right3 = a_right[i+3];
      register float up3 = a_up[i+3];
      register float down3 = a_down[i+3];

      register float bot = a_self[self_idx-1];
      register float self0 = a_self[self_idx];
      register float self1 = a_self[self_idx+1];
      register float self2 = a_self[self_idx+2];
      register float self3 = a_self[self_idx+3];
      register float top = a_self[self_idx+4];

      register float next0 = left0 + right0 + up0 + down0 + bot   + self1;
      register float next1 = left1 + right1 + up1 + down1 + self0 + self2;
      register float next2 = left2 + right2 + up2 + down2 + self1 + self3;
      register float next3 = left3 + right3 + up3 + down3 + self2 + top;
      
      self0 = self0 * c0f;
      self1 = self1 * c0f;
      self2 = self2 * c0f;
      self3 = self3 * c0f;
    
      asm volatile ("fmsub.s %[rd], %[rs1], %[rs2], %[rs3]" \
        : [rd] "=f" (next0) \
        : [rs1] "f" (next0), [rs2] "f" (c1f), [rs3] "f" (self0) \
      );
      asm volatile ("fmsub.s %[rd], %[rs1], %[rs2], %[rs3]" \
        : [rd] "=f" (next1) \
        : [rs1] "f" (next1), [rs2] "f" (c1f), [rs3] "f" (self1) \
      );
      asm volatile ("fmsub.s %[rd], %[rs1], %[rs2], %[rs3]" \
        : [rd] "=f" (next2) \
        : [rs1] "f" (next2), [rs2] "f" (c1f), [rs3] "f" (self2) \
      );
      asm volatile ("fmsub.s %[rd], %[rs1], %[rs2], %[rs3]" \
        : [rd] "=f" (next3) \
        : [rs1] "f" (next3), [rs2] "f" (c1f), [rs3] "f" (self3) \
      );

      Anext[Index3D(nx,ny,nz,__bsg_x,__bsg_y,1+ii+i)] = next0; 
      Anext[Index3D(nx,ny,nz,__bsg_x,__bsg_y,1+ii+i+1)] = next1; 
      Anext[Index3D(nx,ny,nz,__bsg_x,__bsg_y,1+ii+i+2)] = next2; 
      Anext[Index3D(nx,ny,nz,__bsg_x,__bsg_y,1+ii+i+3)] = next3; 
    }

    bsg_barrier_hw_tile_group_sync();

  }
}
