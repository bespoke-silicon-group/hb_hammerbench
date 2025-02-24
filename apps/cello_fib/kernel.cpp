#include <cello/cello.hpp>
#include <util/test_eq.hpp>
#include <util/statics.hpp>
#include "bsg_manycore.h"

DRAM(int) fib_n = 0, fib_result = 0, fib_expect = 0;


int fib(int n)
{
    if (n <= 1) {
        return n;
    } else {
        int x, y, *xp, *yp;
        xp = bsg_tile_group_remote_pointer<int>(__bsg_x, __bsg_y, &x);
        yp = bsg_tile_group_remote_pointer<int>(__bsg_x, __bsg_y, &y);
        cello::parallel_invoke([=]() { *xp = fib(n - 1); },
                               [=]() { *yp = fib(n - 2); });
        return x + y;
    }
}

extern "C" int cello_main(int argc, char *argv[])
{
    bsg_print_int(fib_n);
    bsg_print_int(fib(fib_n));
    fib_result = fib(fib_n);
    TEST_EQ(INT, fib_result, fib_expect);
    return 0;
}
