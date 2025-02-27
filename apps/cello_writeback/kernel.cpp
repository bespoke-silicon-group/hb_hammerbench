#include <cello/cello.hpp>
#include <util/statics.hpp>

int write_back(int depth)
{
    if (depth <= 0) {
        return depth;
    }

    return write_back(depth - 1) + 1;
}

int spawn_and_write_back(int depth)
{
    using namespace cello;
    using joiner = one_child_joiner;

    if (depth <= 0) {
        return depth;
    }

    int wb = 0;
    bsg_global_pointer::pointer<int> wb_ptr = bsg_global_pointer::pointer<int>::onPodXY
        (my::pod_x(), my::pod_y(), (bsg_tile_group_remote_pointer<int>(my::tile_x(), my::tile_y(), &wb)));
    
    bsg_print_int(1000+cello::my::pod_x());
    bsg_print_int(2000+cello::my::pod_y());
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(my::tile_x(), my::tile_y(), &j);
    task *t = new_task([wb_ptr, depth]()mutable{
        *wb_ptr = spawn_and_write_back(depth - 1);
        bsg_print_int(3000+wb_ptr.pod_x());
        bsg_print_int(4000+wb_ptr.pod_y());
    },*jp);
    spawn(t);
    wait(jp);
    return wb + 1;
}

int cello_main(int argc, char *argv[])
{
    //spawn_and_write_back();
    using namespace cello;
#if 1
    // bsg_print_int(cello::my::id());
    // bsg_print_int(cello::my::tile_x());
    // bsg_print_int(cello::my::tile_y());
    // bsg_print_int(cello::my::pod_x());
    // bsg_print_int(cello::my::pod_y());
    // bsg_print_int(cello::my::num_pods());
    bsg_fence();
    int n = 20;
    int wb = spawn_and_write_back(n);
    bsg_print_int(-wb);
    bsg_print_int(-write_back(n));
    //bsg_fence();
#endif
    return 0;
}
