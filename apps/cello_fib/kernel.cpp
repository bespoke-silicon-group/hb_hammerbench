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
        int x, y;
        bsg_global_pointer::reference<int> xg = *cello::addressof_localvar(x);
        bsg_global_pointer::reference<int> yg = *cello::addressof_localvar(y);
        cello::parallel_invoke([=]() mutable {
            xg = fib(n - 1);
        }, [=]() mutable {
            yg = fib(n - 2);
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
