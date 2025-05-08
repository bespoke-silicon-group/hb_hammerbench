#include <cello/cello.hpp>
#include <datastructure/vector.hpp>
#include <util/statics.hpp>
#include "common.hpp"

using namespace cello;

//#define COUNT
//#define TRACE

#define pr(fmt, ...)                            \
    do { bsg_printf("pod[x=%2d,y=%2d], tile[x=%2d,y=%2d]: " fmt, my::pod_x(), my::pod_y(), my::tile_x(), my::tile_y(), ##__VA_ARGS__); } while (0)

DRAM(vector_type) vec_int;

DRAM(std::atomic<int>) counter;

int cello_main(int argc, char *argv[])
{
#ifdef TRACE
    on_every_pod([](){
        pr("vec_int.data() = %p\n", vec_int.data());
        pr("vec_int.size() = %d\n", vec_int.size());
    });
#endif
    vec_int.foreach<cello::parallel>([](int i, int &v) {
        bsg_print_int(i);
        bsg_fence();
        if (i != v)
#ifdef TRACE
            pr("FAIL: vec_int[%2d] = %2d\n", i, v);
#else
            bsg_print_int(-v);
#endif
        v = 2*v;
#ifdef COUNT
        bsg_global_pointer::pod_address paddr;
        paddr.set_pod_x(0).set_pod_y(0);
        bsg_global_pointer::pod_address_guard grd(paddr);
        counter++;
#endif
    });

#ifdef COUNT
    if (counter != vec_int.size()) {
        pr("FAIL: counter = %d, size = %d\n",
           counter.load(),
           vec_int.size());
    }
#endif
    return 0;
}
