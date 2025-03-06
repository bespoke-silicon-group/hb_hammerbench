#include <cello/cello.hpp>

void delegate_scheduler(int to_id) {
    using namespace cello;
    using joiner = n_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
    task *t = new_task([]() {
        bsg_printf("Hello from tile %d: pod_x=%d, pod_y=%d, tile_x=%d, tile_y=%d\n"
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
    bsg_printf("num_tiles_x() = %d\n", my::num_tiles_x());
    bsg_printf("num_tiles_y() = %d\n", my::num_tiles_y());
    bsg_printf("num_tiles() = %d\n", my::num_tiles());
    bsg_printf("num_pods_x() = %d\n", my::num_pods_x());
    bsg_printf("num_pods_y() = %d\n", my::num_pods_y());
    bsg_printf("num_pods() = %d\n", my::num_pods());
    delegate_scheduler(my::id());
    delegate_scheduler(my::id() + 1);
    delegate_scheduler(my::num_tiles() - 1);
    delegate_scheduler(my::num_tiles());
    return 0;
}
