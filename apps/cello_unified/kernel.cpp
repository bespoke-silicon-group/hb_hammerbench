#include <cello/cello.hpp>
#include <util/statics.hpp>
#include "data.hpp"

DRAM(data_t) data;

#define pr(fmt, ...)                            \
    bsg_printf("pod=(%d,%d): tile=(%d,%d): " fmt , cello::my::pod_x(), cello::my::pod_y(), cello::my::tile_x(), cello::my::tile_y(), ##__VA_ARGS__)

int cello_main(int argc, char *argv[])
{
    cello::on_every_pod([]() {
        pr("data{ .a()=%2d, .b()=%2d, .c()=%2d, .d() = %2d }\n", data.a(), data.b(), data.c(), data.d());
        data.a() = 5;
        data.b() = 6;
        data.c() = 7;
        data.d() = 8;
        pr("data{ .i() = { .x()=%2d, .y() = %2d } }\n", data.i().x(), data.i().y());
    });
    return 0;
}
