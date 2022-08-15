//This kernel adds 2 vectors

#define BSG_TILE_GROUP_X_DIM bsg_tiles_X
#define BSG_TILE_GROUP_Y_DIM bsg_tiles_Y
#include <math.h>
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_group_strider.hpp"
#include "bsg_cuda_lite_barrier.h"


#define bsg_tg_float_ptr(x,y,addr) \
  ((float *) ((1 << REMOTE_PREFIX_SHIFT) \
             | (y << REMOTE_Y_CORD_SHIFT) \
             | (x << REMOTE_X_CORD_SHIFT) \
             | ((int) addr)))


float a_self[LOCAL_SIZE+2]    = {0.0f};
float zero_buffer[LOCAL_SIZE] = {0.0f};


extern "C" void compute(
  int c0, int c1,
  float * bsg_attr_noalias A0,
  float * bsg_attr_noalias Anext,
  float * bsg_attr_noalias a_left,
  float * bsg_attr_noalias a_right,
  float * bsg_attr_noalias a_up,
  float * bsg_attr_noalias a_down,
  float * bsg_attr_noalias a_self,
  const int nx,
  const int ny,
  const int nz
);


// Main Kernel Function
extern "C" __attribute__ ((noinline)) __attribute__((used))
int kernel_jacobi(
  int c0, int c1,
  float *A0, float * Anext,
  const int nx, const int ny, const int nz, const int nw) {

  const bool x_l_bound = (__bsg_x == 0);
  const bool x_h_bound = (__bsg_x == (bsg_tiles_X-1));
  const bool y_l_bound = (__bsg_y == 0);
  const bool y_h_bound = (__bsg_y == (bsg_tiles_Y-1));

  bsg_barrier_hw_tile_group_init();
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();


  bsg_cuda_print_stat_kernel_start(); // stat kernel start

  // Construct remote pointers
  float* a_up, *a_down, *a_left, *a_right;

  if (x_l_bound) {
    a_left = zero_buffer;
  } else {
    a_left = bsg_tg_float_ptr(__bsg_x-1, __bsg_y, &a_self[1]);
  }
  if (x_h_bound) {
    a_right = zero_buffer;
  } else {
    a_right = bsg_tg_float_ptr(__bsg_x+1, __bsg_y, &a_self[1]);
  }
  if (y_l_bound) {
    a_down = zero_buffer;
  } else {
    a_down = bsg_tg_float_ptr(__bsg_x, __bsg_y-1, &a_self[1]);
  }
  if (y_h_bound) {
    a_up   = zero_buffer;
  } else {
    a_up   = bsg_tg_float_ptr(__bsg_x, __bsg_y+1, &a_self[1]);
  }



  for (int i = 0; i < nw; i++) {
    int idx = nx*ny*nz*i; 
    compute(c0, c1,
      &A0[idx], &Anext[idx],
      a_left, a_right, a_up, a_down, a_self,
      nx, ny, nz);
  }
  
  bsg_cuda_print_stat_kernel_end(); // stat kernel end
  bsg_fence();
  bsg_barrier_hw_tile_group_sync();

	return 0;
}
