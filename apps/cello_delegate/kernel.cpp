#include <cello/cello.hpp>

void delegate_scheduler(int to_id) {
    using namespace cello;
    using joiner = n_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
    task *t = new_task([]() {
        bsg_printf("Hello from tile %2d: pod_x=%2d, pod_y=%2d, tile_x=%2d, tile_y=%2d\n"
                   , my::id()
                   , my::pod_x()
                   , my::pod_y()
                   , my::tile_x()
                   , my::tile_y()
                   );
    }, *jp);
    delegate(to_id, t);
    wait(&j);
    delete t;
}

int cello_main(int argc, char **argv) {
    using namespace cello;
    delegate_scheduler(my::id());
    delegate_scheduler(my::id() + 1);
    delegate_scheduler(my::num_tiles() - 1);
    delegate_scheduler(my::num_tiles());
    delegate_scheduler(my::num_tiles_total() - 1);
    return 0;
}
