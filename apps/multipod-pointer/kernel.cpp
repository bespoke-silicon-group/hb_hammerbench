#include "global_pointer/global_pointer.hpp"
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_cuda_lite_barrier.h"
#include <array>

using namespace bsg_global_pointer;

__attribute__((section(".dram"))) unsigned g_pod_x = 0;
__attribute__((section(".dram"))) unsigned g_pod_y = 0;
__attribute__((section(".dram"))) unsigned g_special = 0;
__attribute__((section(".dram"))) std::array<unsigned, 4> g_array;
__attribute__((section(".dram"))) pointer<unsigned> g_ptr;

struct foo {
        FIELD(unsigned, x);
        FIELD(unsigned, y);
        unsigned sum() const { return x() + y(); }
};

__attribute__((section(".dram"))) foo g_foo;

namespace bsg_global_pointer
{
        template <>
        class reference<foo> {
                BSG_GLOBAL_POINTER_REFERENCE_TRIVIAL(foo);
                BSG_GLOBAL_POINTER_REFERENCE_FIELD(foo, x);
                BSG_GLOBAL_POINTER_REFERENCE_FIELD(foo, y);
                BSG_GLOBAL_POINTER_REFERENCE_METHOD_CONST(foo, sum, unsigned);
        };
}

#define UDEC unsigned, "%u"
#define UHEX unsigned, "%x"

#define NEWV -1
#define OLDV 0xcafebabe
#define BADV 0xdeadbeef

void bsg_print_str(const char *str)
{
        while (*str) { bsg_putchar(*str++); }
}

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

#define _TEST_EQ(type, fmt, expr, expect)                               \
        do {                                                            \
                type e = (expr);                                        \
                if (e != expect) {                                      \
                        bsg_print_str(__FILE__ ":" STRINGIFY(__LINE__) ": FAIL:  "#expr " != " #expect "\n"); \
                }  else {                                               \
                        bsg_print_str(__FILE__ ":" STRINGIFY(__LINE__) ": PASS:  "#expr " == " #expect "\n"); \
                }                                                       \
        } while (0)

#define _TEST_EQ_SAVE(type, fmt, save, expr, expect)                    \
        do {                                                            \
                type e = (expr);                                        \
                save = e;                                               \
                if (e != expect) {                                      \
                        bsg_print_str(__FILE__ ":" STRINGIFY(__LINE__) ": FAIL:  "#expr " != " #expect "\n"); \
                } else {                                                \
                        bsg_print_str(__FILE__ ":" STRINGIFY(__LINE__) ": PASS:  "#expr " == " #expect "\n"); \
                }                                                       \
        } while (0)

#define TEST_EQ(typefmt, expr, expect) _TEST_EQ(typefmt, expr, expect)
#define TEST_EQ_SAVE(typefmt, save, expr, expect) _TEST_EQ_SAVE(typefmt, save, expr, expect)

extern "C" int setup(unsigned pod_x, unsigned pod_y)
{
        bsg_barrier_tile_group_init();
        bsg_barrier_tile_group_sync();
        {
                g_pod_x = pod_x;
                g_pod_y = pod_y;
                g_foo.x() = pod_x;
                g_foo.y() = pod_y;
                if (pod_x == 0 && pod_y == 0) {
                        g_special = OLDV;
                        for (unsigned i = 0; i < g_array.size(); i++) {
                                g_array[i] = i;
                        }
                        g_ptr = pointer<unsigned>(&g_special);
                } else {
                        g_special = BADV;
                }
        }
        bsg_barrier_tile_group_sync();
        return 0;
}

/*
 * test the pod_address and pod_address_guard classes
 */
extern "C" int multipod_pointer_t0()
{
        bsg_barrier_tile_group_init();
        bsg_barrier_tile_group_sync();
        pod_address addr{0};
        addr.set_pod_x(0).set_pod_y(0);
        {
                pod_address_guard grd(addr);
                TEST_EQ(UDEC, g_pod_x, 0);
                TEST_EQ(UDEC, g_pod_y, 0);
                TEST_EQ(UHEX, g_special, OLDV);
        }
        bsg_barrier_tile_group_sync();
        return 0;
}

/*
 * test the address class
 */
extern "C" int multipod_pointer_t1()
{
        bsg_barrier_tile_group_init();
        bsg_barrier_tile_group_sync();
        {
                address a_of_special(&g_special);
                a_of_special.set_pod_x(0).set_pod_y(0);

                unsigned old;
                TEST_EQ_SAVE(UHEX, old, a_of_special.read<unsigned>(), OLDV);
                TEST_EQ(UHEX, g_special, 0xdeadbeef);

                a_of_special.write<unsigned>(NEWV);
                TEST_EQ(UHEX, a_of_special.read<unsigned>(), NEWV);
                TEST_EQ(UHEX, g_special, 0xdeadbeef);

                a_of_special.write<unsigned>(old);
                TEST_EQ(UHEX, a_of_special.read<unsigned>(), OLDV);

        }
        bsg_barrier_tile_group_sync();
        return 0;
}

/*
 * test the reference class
 */
extern "C" int multipod_pointer_t2()
{
        bsg_barrier_tile_group_init();
        bsg_barrier_tile_group_sync();
        {
                reference<unsigned> r_of_special(&g_special);
                r_of_special.set_pod_x(0).set_pod_y(0);

                unsigned old;
                TEST_EQ_SAVE(UHEX, old, r_of_special.read(), OLDV);
                TEST_EQ(UHEX, g_special, BADV);

                r_of_special = NEWV;
                TEST_EQ(UHEX, r_of_special, NEWV);
                TEST_EQ(UHEX, g_special, BADV);
                
                r_of_special = old;
                TEST_EQ(UHEX, r_of_special, OLDV);
        }
        bsg_barrier_tile_group_sync();
        return 0;
}

/*
 * test the pointer class
 */
extern "C" int multipod_pointer_t3()
{
        bsg_barrier_tile_group_init();
        bsg_barrier_tile_group_sync();
        {
                pointer<unsigned> p_of_special(&g_special);
                p_of_special.set_pod_x(0).set_pod_y(0);

                unsigned old;
                TEST_EQ_SAVE(UHEX, old, *p_of_special, OLDV);
                TEST_EQ(UHEX, g_special, BADV);

                *p_of_special = NEWV;
                TEST_EQ(UHEX, *p_of_special, NEWV);
                TEST_EQ(UHEX, g_special, BADV);

                *p_of_special = old;
                TEST_EQ(UHEX, *p_of_special, OLDV);

                pointer<unsigned> p_of_array(&g_array[0]);
                p_of_array.set_pod_x(0).set_pod_y(0);
                TEST_EQ(UDEC, p_of_array[0], 0);
                TEST_EQ(UDEC, p_of_array[1], 1);
                TEST_EQ(UDEC, p_of_array[2], 2);
                TEST_EQ(UDEC, p_of_array[3], 3);

                pointer<pointer<unsigned>> p_of_ptr(&g_ptr);
                p_of_ptr.set_pod_x(0).set_pod_y(0);
                TEST_EQ(UHEX, **p_of_ptr, OLDV);

                pointer<foo> p_of_foo(&g_foo);
                p_of_foo.set_pod_x(0).set_pod_y(0);
                TEST_EQ(UDEC, p_of_foo->x(), 0);
                TEST_EQ(UDEC, p_of_foo->y(), 0);
                TEST_EQ(UDEC, p_of_foo->sum(), 0);
        }
        bsg_barrier_tile_group_sync();
        return 0;
}
