#include <cello/cello.hpp>
#include <util/statics.hpp>

DRAM(int) data[NITERS];

int cello_main(int argc, char *argv[])
{
    using joiner = cello::n_child_joiner;
    joiner j;
    joiner*jp = bsg_tile_group_remote_pointer<joiner>
        (cello::my::tile_x(), cello::my::tile_y(), &j);

    cello::parallel_foreach(0, NITERS, [jp](int i) {
        cello::on_every_pod(jp, [i]() {
            bsg_print_int(i);
            data[i] = 1;
        });
    });

    cello::wait(jp);
    return 0;
}
