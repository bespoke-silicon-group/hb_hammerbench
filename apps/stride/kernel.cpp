#include "bsg_manycore.h"
#include "bsg_cuda_lite_barrier.h"

template <typename T>
static inline void phex(T v) {
    bsg_print_hexadecimal(reinterpret_cast<unsigned>(v));
}

template <typename T>
static inline void pint(T v) {
    bsg_print_int(static_cast<int>(v));
}

static inline int my_x() {
    return __bsg_x;
}

static inline int my_y() {
    return __bsg_y;
}

static inline int my_id() {
    return my_y() * bsg_tiles_X + my_x();
}

static inline int threads() {
    return bsg_tiles_X * bsg_tiles_Y;
}

extern "C" int warmup(int *A, int s, int n) {
    bsg_barrier_hw_tile_group_init();
    bsg_barrier_hw_tile_group_sync();
    for (int i = my_id(), j = s*my_id(); i < n; i += threads(), j += s*threads()) {
        asm volatile ("lw x0, %0" : : "m"(A[j]) : "memory");
    }
    bsg_barrier_hw_tile_group_sync();
    return 0;
}

extern "C" int stride(int *A, int s, int n, int x, int y) {
    bsg_barrier_hw_tile_group_init();
    bsg_barrier_hw_tile_group_sync();
    bsg_cuda_print_stat_kernel_start();
    int sum = 0;
    if (my_x() == x && my_y() == y) {
        for (int i = 0, j = 0; i < n; i += 1, j += s) {
            sum += A[j];
        }
    }
    bsg_cuda_print_stat_kernel_end();
    bsg_barrier_hw_tile_group_sync();
    return sum;
}
