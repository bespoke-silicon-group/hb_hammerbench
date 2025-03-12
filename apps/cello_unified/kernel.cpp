#include <cello/cello.hpp>
#include <util/statics.hpp>
#include "data.hpp"

DRAM(data_t) data;

#define pr(fmt, ...)                            \
    bsg_printf("pod=(%d,%d): tile=(%d,%d): " fmt "\n", cello::my::pod_x(), cello::my::pod_y(), cello::my::tile_x(), cello::my::tile_y(), ##__VA_ARGS__)

int cello_main(int argc, char *argv[])
{
    cello::on_every_pod([]() {
        pr("data{ .a()=%2d, .b()=%2d, .c()=%2d, .d() = %2d }", data.a(), data.b(), data.c(), data.d());
    });
    return 0;
}
