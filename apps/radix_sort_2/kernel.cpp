#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_group_strider.hpp"
#include "bsg_cuda_lite_barrier.h"
#include <cstdint>

// #define NUM_BITS  4
#define NUM_BUCKETS (1 << NUM_BITS)

int local_cnt [NUM_BUCKETS];
int histogram [NUM_BUCKETS];
int my_offset [NUM_BUCKETS];

inline uint32_t get_digits(uint32_t val, int bit_ofst){
  return (val >> bit_ofst) & (NUM_BUCKETS - 1);
}

void counting_sort(uint32_t *A, uint32_t *buffer, int N, int bit_ofst) {
  int tid = __bsg_id;
  int tile_cnt = bsg_tiles_X * bsg_tiles_Y;
  int psize = (N + tile_cnt - 1) / tile_cnt;

  for (int i = 0; i < NUM_BUCKETS; i++) {
    local_cnt[i] = 0;
    histogram[i] = 0;
    my_offset[i] = 0;
  }
  bsg_barrier_tile_group_sync();

                                                                                    // --- DEBUG: print N, psize, tile_cnt ---
                                                                                    if (DEBUG_PRINT ) {
                                                                                      if (tid == 0) {
                                                                                        bsg_print_int(-1);      // separator
                                                                                        bsg_print_int(tid);
                                                                                        bsg_print_int(N);
                                                                                        bsg_print_int(psize);
                                                                                        bsg_print_int(tile_cnt);
                                                                                      }
                                                                                      bsg_barrier_tile_group_sync();
                                                                                    }

  // 1. generate histogram
#if UNROLL
  bsg_unroll(8)
#endif
  for (int i = 0; i < psize; i++) {
    int idx = i + tid * psize;
    if (idx >= N) break;
    local_cnt[get_digits(A[idx], bit_ofst)]++;
  }
  bsg_barrier_tile_group_sync();

                                                                                  // --- DEBUG: print local_cnt for first 4 tiles ---
                                                                                  if (DEBUG_PRINT) {
                                                                                    if (tid == 0) {
                                                                                      bsg_print_int(-2);      // separator: "local_cnt"
                                                                                      bsg_print_int(tid);
                                                                                      for (int i = 0; i < NUM_BUCKETS; i++) {
                                                                                        bsg_print_int(local_cnt[i]);
                                                                                      }
                                                                                    }
                                                                                    bsg_barrier_tile_group_sync();
                                                                                    if (tid == 1) {
                                                                                      bsg_print_int(-2);      // separator: "local_cnt"
                                                                                      bsg_print_int(tid);
                                                                                      for (int i = 0; i < NUM_BUCKETS; i++) {
                                                                                        bsg_print_int(local_cnt[i]);
                                                                                      }
                                                                                    }
                                                                                    bsg_barrier_tile_group_sync();
                                                                                    if (tid == 2) {
                                                                                      bsg_print_int(-2);      // separator: "local_cnt"
                                                                                      bsg_print_int(tid);
                                                                                      for (int i = 0; i < NUM_BUCKETS; i++) {
                                                                                        bsg_print_int(local_cnt[i]);
                                                                                      }
                                                                                    }
                                                                                    bsg_barrier_tile_group_sync();
                                                                                    if (tid == 3) {
                                                                                      bsg_print_int(-2);      // separator: "local_cnt"
                                                                                      bsg_print_int(tid);
                                                                                      for (int i = 0; i < NUM_BUCKETS; i++) {
                                                                                        bsg_print_int(local_cnt[i]);
                                                                                      }
                                                                                    }
                                                                                    bsg_barrier_tile_group_sync();
                                                                                  }

  // 2. merge histogram
  if (__bsg_x == 0) {
#if UNROLL
  bsg_unroll(8)
#endif
    for (int i = 0; i < NUM_BUCKETS; ++i) {
      histogram[i] = local_cnt[i];
#if UNROLL
  bsg_unroll(8)
#endif
      for (int x = 1; x < bsg_tiles_X; ++x) {
        histogram[i] += *bsg_remote_ptr(x, __bsg_y, &local_cnt[i]);
      }
    }
  }
  bsg_barrier_tile_group_sync();

  if (tid == 0) {
#if UNROLL
  bsg_unroll(8)
#endif
    for (int i = 0; i < NUM_BUCKETS; ++i) {
#if UNROLL
  bsg_unroll(8)
#endif
      for (int y = 1; y < bsg_tiles_Y; ++y) {
        histogram[i] += *bsg_remote_ptr(0, y, &histogram[i]);
      }
    }
  }
  bsg_barrier_tile_group_sync();

                                                                                  // --- DEBUG: merged histogram before prefix sum ---
                                                                                    if (DEBUG_PRINT) {
                                                                                      if (tid == 0) {
                                                                                        bsg_print_int(-3);      // separator: "merged histogram"
                                                                                        bsg_print_int(tid);
                                                                                        for (int i = 0; i < NUM_BUCKETS; i++) {
                                                                                          bsg_print_int(histogram[i]);
                                                                                        }
                                                                                      }
                                                                                      bsg_barrier_tile_group_sync();
                                                                                    }

  // 3. prefix sum (exclusive), tid==0 only
  if (tid == 0) {
      int prev = 0;
#if UNROLL
  bsg_unroll(8)
#endif
      for (int i = 0; i < NUM_BUCKETS; i++) {
          int tmp = histogram[i];  // 保存原始频率
          histogram[i] = prev;     // 写入前缀和
          prev += tmp;             // 累加原始频率
      }
  }
  bsg_barrier_tile_group_sync();

  // broadcast histogram to all tiles
  if (__bsg_x == 0 && __bsg_y != 0) {
#if UNROLL
  bsg_unroll(8)
#endif
    for (int i = 0; i < NUM_BUCKETS; ++i) {
      histogram[i] = *bsg_remote_ptr(0, 0, &histogram[i]);
    }
  }
  bsg_barrier_tile_group_sync();
  if (__bsg_x != 0) {
#if UNROLL
  bsg_unroll(8)
#endif
    for (int i = 0; i < NUM_BUCKETS; ++i) {
      histogram[i] = *bsg_remote_ptr(0, __bsg_y, &histogram[i]);
    }
  }
  bsg_barrier_tile_group_sync();

                                                                                  // --- DEBUG: histogram after prefix sum ---
                                                                                    if (DEBUG_PRINT ) {
                                                                                      if (tid == 0) {
                                                                                        bsg_print_int(-4);      // separator: "prefix sum histogram"
                                                                                        bsg_print_int(tid);
                                                                                        for (int i = 0; i < NUM_BUCKETS; i++) {
                                                                                          bsg_print_int(histogram[i]);
                                                                                        }
                                                                                      }
                                                                                      bsg_barrier_tile_group_sync();
                                                                                    }

  // 4a. compute my_offset
#if UNROLL
  bsg_unroll(8)
#endif
  for (int i = 0; i < NUM_BUCKETS; i++) {
    my_offset[i] = 0;
#if UNROLL
  bsg_unroll(8)
#endif
    for (int t = 0; t < tid; t++) {
      int tx = t % bsg_tiles_X;
      int ty = t / bsg_tiles_X;
      my_offset[i] += *bsg_remote_ptr(tx, ty, &local_cnt[i]);
    }
  }
  bsg_barrier_tile_group_sync();

                                                                                // --- DEBUG: my_offset for first 4 tiles ---
                                                                                if (DEBUG_PRINT) {
                                                                                  if (tid == 0) {
                                                                                    bsg_print_int(-5);      // separator: "my_offset"
                                                                                    bsg_print_int(tid);
                                                                                    for (int i = 0; i < NUM_BUCKETS; i++) {
                                                                                      bsg_print_int(my_offset[i]);
                                                                                    }
                                                                                  }
                                                                                  bsg_barrier_tile_group_sync();
                                                                                  if (tid == 1) {
                                                                                    bsg_print_int(-5);      // separator: "my_offset"
                                                                                    bsg_print_int(tid);
                                                                                    for (int i = 0; i < NUM_BUCKETS; i++) {
                                                                                      bsg_print_int(my_offset[i]);
                                                                                    }
                                                                                  }
                                                                                  bsg_barrier_tile_group_sync();
                                                                                  if (tid == 2) {
                                                                                    bsg_print_int(-5);      // separator: "my_offset"
                                                                                    bsg_print_int(tid);
                                                                                    for (int i = 0; i < NUM_BUCKETS; i++) {
                                                                                      bsg_print_int(my_offset[i]);
                                                                                    }
                                                                                  }
                                                                                  bsg_barrier_tile_group_sync();
                                                                                  if (tid == 3) {
                                                                                    bsg_print_int(-5);      // separator: "my_offset"
                                                                                    bsg_print_int(tid);
                                                                                    for (int i = 0; i < NUM_BUCKETS; i++) {
                                                                                      bsg_print_int(my_offset[i]);
                                                                                    }
                                                                                  }
                                                                                  bsg_barrier_tile_group_sync();
                                                                                }

  // 4b. scatter to buffer
#if UNROLL
  bsg_unroll(8)
#endif
  for (int i = 0; i < psize; i++) {
    int idx = i + tid * psize;
    if (idx >= N) break;
    uint32_t val = A[idx];
    uint32_t digit = get_digits(val, bit_ofst);
    int pos = histogram[digit] + my_offset[digit];
    buffer[pos] = val;
    my_offset[digit]++;
  }
  bsg_barrier_tile_group_sync();

                                                                                    if (DEBUG_PRINT ) {
                                                                                      if (tid == 0) {
                                                                                        bsg_print_int(-6);      // separator: "buffer after scatter"
                                                                                        bsg_print_int(tid);
                                                                                        for (int i = 0; i < N; i++) {
                                                                                          bsg_print_int((int)buffer[i]);
                                                                                        }
                                                                                      }
                                                                                      bsg_barrier_tile_group_sync();
                                                                                    }

  // 5. copy back
#if UNROLL
  bsg_unroll(8)
#endif
  for (int i = tid; i < N; i += tile_cnt) {
    A[i] = buffer[i];
  }
  bsg_barrier_tile_group_sync();
}

extern "C" __attribute__ ((noinline))
int
kernel_radix_sort(uint32_t *A, uint32_t *buffer, int N) {

  bsg_barrier_tile_group_init();
  bsg_barrier_tile_group_sync();
  bsg_cuda_print_stat_kernel_start();


  for (int i = 0; i < 32; i += NUM_BITS) {
    counting_sort(A, buffer, N, i);
    bsg_barrier_tile_group_sync();
  }

  bsg_fence();
  bsg_cuda_print_stat_kernel_end();
  bsg_fence();
  bsg_barrier_tile_group_sync();

  return 0;
}
