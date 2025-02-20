#include <cello/cello.hpp>
#include <bsg_manycore.h>

extern "C" int setup(unsigned pod_x, unsigned pod_y)
{
    return 0;
}

void recurse_scheduler(int depth)
{
    using namespace cello;
    bsg_print_int(depth);
    if (depth == 0) {
        return;
    } else {
        joiner j;
        joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
        task *t = new_task([=]() { recurse_scheduler(depth - 1); }, *jp);
        spawn(t);
        recurse_scheduler(depth - 1);
        wait(jp);
    }
}

void recurse_invoke(int depth)
{
    using namespace cello;
    bsg_print_int(depth);
    if (depth == 0) {
        return;
    } else {
        parallel_invoke([=]() { recurse_invoke(depth - 1); },
                        [=]() { recurse_invoke(depth - 1); });
    }
}

int cello_main(int argc, char *argv[])
{
    using namespace cello;
    recurse_invoke(6);
    return 0;
}
