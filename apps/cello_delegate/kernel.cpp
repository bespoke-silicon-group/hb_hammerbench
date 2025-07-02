#include <cello/cello.hpp>
#include <array>

struct functor {
    void operator()() {}
    std::array<unsigned, TASK_SIZE/sizeof(unsigned)> payload = {};
};

int cello_main(int argc, char *argv[])
{
    using namespace cello;
    using joiner = n_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(my::tile_x(), my::tile_y(), &j);
    for (int i = 0; i < N; i++) {
        int pod_id = i % my::num_pods();
        on_pod(pod_id, jp, functor{});
    }
    wait(jp);    
    return 0;
}
