#include <cello/cello.hpp>
#include <datastructure/vector.hpp>
#include <util/statics.hpp>

using namespace cello;

#define pr(fmt, ...)                            \
    do { bsg_printf("pod[x=%2d,y=%2d], tile[x=%2d,y=%2d]: " fmt, my::pod_x(), my::pod_y(), my::tile_x(), my::tile_y(), ##__VA_ARGS__); } while (0)

DRAM(datastructure::vector<int>) vec_int;

int cello_main(int argc, char *argv[])
{
    on_every_pod([](){
        pr("vec_int.data() = %p\n", vec_int.data());
        pr("vec_int.size() = %d\n", vec_int.size());
    });
    vec_int.foreach([](int i, int &v) {
        if (i != v)
            pr("FAIL: vec_int[%2d] = %2d\n", i, v);
        v = 2*v;
    });
    return 0;
}
