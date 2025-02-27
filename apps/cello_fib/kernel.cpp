#include <cello/cello.hpp>
#include <util/test_eq.hpp>
#include <util/statics.hpp>
#include <global_pointer/global_pointer.hpp>
#include "bsg_manycore.h"

DRAM(int) fib_n = 0, fib_result = 0, fib_expect = 0;


int fib(int n)
{
    if (n <= 1) {
        return n;
    } else {
        int x, y, *xp, *yp;
        xp = bsg_tile_group_remote_pointer<int>(cello::my::tile_x(), cello::my::tile_y(), &x);
        yp = bsg_tile_group_remote_pointer<int>(cello::my::tile_x(), cello::my::tile_y(), &y);
        bsg_global_pointer::pointer<int> xg, yg;
        xg = bsg_global_pointer::pointer<int>::onPodXY(cello::my::pod_x(), cello::my::pod_y(), xp);
        yg = bsg_global_pointer::pointer<int>::onPodXY(cello::my::pod_x(), cello::my::pod_y(), yp);
        cello::parallel_invoke([=]() mutable {
            *xg = fib(n - 1);
        }, [=]() mutable {
            *yg = fib(n - 2);
        });
        return x+y;
    }
}

extern "C" int cello_main(int argc, char *argv[])
{
    bsg_print_int(fib_n);
    fib_result = fib(fib_n);
    //TEST_EQ(INT, fib_result, fib_expect);
    bsg_print_int(fib_result);
    bsg_print_int(fib_expect);
    return 0;
}
