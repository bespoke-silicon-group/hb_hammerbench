#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_cuda_lite_barrier.h"

#ifndef BSG_COORD_X_WIDTH
#error "BSG_COORD_X_WIDTH is not defined"
#endif
#ifndef BSG_COORD_Y_WIDTH
#error "BSG_COORD_Y_WIDTH is not defined"
#endif
#ifndef BSG_POD_TILES_X
#error "BSG_POD_X is not defined"
#endif
#ifndef BSG_POD_TILES_Y
#error "BSG_POD_Y is not defined"
#endif
#ifndef BSG_PODS_X
#error "BSG_PODS_X is not defined"
#endif
#ifndef BSG_PODS_Y
#error "BSG_PODS_Y is not defined"
#endif

/**
 * the number of bits to represent positive integer x
 */
template <unsigned x>
static inline constexpr unsigned bits() {
        constexpr unsigned y = x >> 1;
        return x == 0 ? 0 : 1 + bits<y>();
}

/**
 * the mask of width bits
 */
template <unsigned width>
static inline constexpr unsigned mask() {
        return (1 << width) - 1;
}

static constexpr unsigned tile_x_width = bits<BSG_POD_TILES_X-1>();
static constexpr unsigned tile_y_width = bits<BSG_POD_TILES_Y-1>();
static constexpr unsigned pod_x_width = BSG_COORD_X_WIDTH - tile_x_width;
static constexpr unsigned pod_y_width = BSG_COORD_Y_WIDTH - tile_y_width;


// unsigned set_pod_addr() {
//         unsigned addr = 0xdeadbeef;
//         asm volatile ("csrw 0x360, %0" : : "r"(addr));
//         return addr;
// }


struct pod_addr {
public:
        pod_addr(unsigned raw)
                : raw(raw) {
        }
        unsigned pod_x() const {
                return phys_pod_x()-1;
        }
        unsigned pod_y() const {
                return (phys_pod_y()-1)>>1;
        }
        unsigned phys_pod_x() const {
                return raw & mask<pod_x_width>();
        }
        unsigned phys_pod_y() const {
                return (raw >> pod_x_width) & mask<pod_y_width>();
        }
        pod_addr & set_phys_pod_x(unsigned x) {
                raw = (raw & ~mask<pod_x_width>()) | (x & mask<pod_x_width>());
                return *this;
        }
        pod_addr & set_phys_pod_y(unsigned y) {
                raw = (raw & ~(mask<pod_y_width>() << pod_x_width)) | ((y & mask<pod_y_width>()) << pod_x_width);
                return *this;
        }
        pod_addr & set_pod_x(unsigned x) {
                return set_phys_pod_x(x+1);
        }
        pod_addr & set_pod_y(unsigned y) {
                return set_phys_pod_y((y<<1)+1);
        }
        unsigned raw;
};

pod_addr get_pod_addr() {
        pod_addr addr(0);
        asm volatile ("csrr %0, 0x360" : "=r"(addr));
        return addr;
}

void set_pod_addr(pod_addr addr) {
        asm volatile ("csrw 0x360, %0" : : "r"(addr.raw));
}

__attribute__((section(".dram"))) unsigned g_pod_x = 0;
__attribute__((section(".dram"))) unsigned g_pod_y = 0;

extern "C" int setup(unsigned pod_x, unsigned pod_y)
{
        bsg_barrier_tile_group_init();
        bsg_barrier_tile_group_sync();
        if (__bsg_id == 0) {
                g_pod_x = pod_x;
                g_pod_y = pod_y;
                asm volatile ("" ::: "memory");
                bsg_fence();
                bsg_print_unsigned(g_pod_x);
                bsg_print_unsigned(g_pod_y);
                bsg_fence();
        }
        bsg_barrier_tile_group_sync();
        return 0;
}

extern "C" int multipod_pointer(unsigned pod_x, unsigned pod_y)
{
        bsg_barrier_tile_group_init();
        bsg_barrier_tile_group_sync();
        if (__bsg_id == 0) {
                pod_addr addr{0};
                addr.set_pod_x(0).set_pod_y(0);
                set_pod_addr(addr);
                bsg_print_unsigned(g_pod_x);
                bsg_print_unsigned(g_pod_y);
                bsg_fence();
        }
        bsg_barrier_tile_group_sync();
        return 0;
}
