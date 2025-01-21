#include <bsg_manycore.h>
#include <bsg_cuda_lite_barrier.h>
#include "bsg_barrier_multipod.h"
#include "coremark.h"
#include <cstdint>

// result struct;
core_results results;
// static mem block;
ee_u8 static_memblk[TOTAL_DATA_SIZE];


void *iterate(void *pres) {
  ee_u32 i;
  ee_u16 crc;
  core_results *res=(core_results *)pres;
  ee_u32 iterations=res->iterations;
  res->crc=0;
  res->crclist=0;
  res->crcmatrix=0;
  res->crcstate=0;

  for (i=0; i<iterations; i++) {
    crc=core_bench_list(res,1);
    res->crc=crcu16(crc,res->crc);
    crc=core_bench_list(res,-1);
    res->crc=crcu16(crc,res->crc);
    if (i==0) res->crclist=res->crc;
  }
  return NULL;
}

// kernal main function;
extern "C"
int kernel(uint16_t *crc, int niter)
{
  bsg_barrier_tile_group_init();
  bsg_barrier_tile_group_sync();

  // setup;
  results.seed1=0;
  results.seed2=0;
  results.seed3=0x66;
  results.iterations=niter;
  results.execs=ALL_ALGORITHMS_MASK;
  results.err=0;
  results.size=TOTAL_DATA_SIZE;
  results.memblock[0]=(void*) static_memblk;

  // assign pointers;
  results.size = results.size/NUM_ALGORITHMS;
  for (int i = 0; i<NUM_ALGORITHMS; i++) {
    results.memblock[i+1]=(char *)(results.memblock[0])+(results.size*i);
  }

  // call inits;
  results.list=core_list_init(results.size, (list_head*) results.memblock[1],results.seed1);
  core_init_matrix(results.size, results.memblock[2], (ee_s32) results.seed1 | (((ee_s32)results.seed2) << 16), &(results.mat));
  core_init_state(results.size,results.seed1,(ee_u8*) results.memblock[3]);

  // start kernel;
  //bsg_barrier_multipod(pod_id, NUM_POD_X, done, &alert);
  bsg_barrier_tile_group_sync();
  bsg_cuda_print_stat_kernel_start();

  // Run Coremark;
  iterate(&results);

  // end kernel;
  bsg_fence();
  bsg_barrier_tile_group_sync();
  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_tile_group_sync();

  // write crc results;
  crc[(__bsg_id*3)+0] = results.crclist;
  crc[(__bsg_id*3)+1] = results.crcmatrix;
  crc[(__bsg_id*3)+2] = results.crcstate;
  bsg_fence();
  bsg_barrier_tile_group_sync();
  return 0;
}
