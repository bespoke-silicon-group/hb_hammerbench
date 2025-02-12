#include "global_pointer/global_pointer.hpp"
#include "bsg_manycore.h"
#include "bsg_set_tile_x_y.h"
#include "bsg_cuda_lite_barrier.h"


using namespace bsg_global_pointer;

__attribute__((section(".dram"))) unsigned g_pod_x = 0;
__attribute__((section(".dram"))) unsigned g_pod_y = 0;
__attribute__((section(".dram"))) unsigned g_special = 0;

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

                if (pod_x == 0 && pod_y == 0) {
                        g_special = OLDV;
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
                address_ext ext;
                ext.pod_addr().set_pod_x(0).set_pod_y(0);

                address a_of_special(ext, &g_special);
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
                address_ext ext;
                ext.pod_addr().set_pod_x(0).set_pod_y(0);

                address a_of_special(ext, &g_special);                
                reference<unsigned> r_of_special(a_of_special);
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
                address_ext ext;
                ext.pod_addr().set_pod_x(0).set_pod_y(0);
                address a_of_special(ext, &g_special);
                pointer<unsigned> p_of_special(a_of_special);

                unsigned old;
                TEST_EQ_SAVE(UHEX, old, *p_of_special, OLDV);
                TEST_EQ(UHEX, g_special, BADV);

                *p_of_special = NEWV;
                TEST_EQ(UHEX, *p_of_special, NEWV);
                TEST_EQ(UHEX, g_special, BADV);

                *p_of_special = old;
                TEST_EQ(UHEX, *p_of_special, OLDV);
        }
        bsg_barrier_tile_group_sync();
        return 0;
}
