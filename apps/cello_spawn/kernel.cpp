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
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
    using ftask = functor_task<functor, joiner>;
    bsg_print_int(sizeof(ftask));
#ifdef ALLOCATE_TASK_IN_DMEM
    char buf[sizeof(ftask)*N];
    ftask *tv = reinterpret_cast<ftask*>(buf);
    tv = bsg_tile_group_remote_pointer<ftask>(__bsg_x, __bsg_y, tv);
#endif
    for (int i = 0; i < N; i++) {
#ifdef ALLOCATE_TASK_IN_DMEM
        task *t = new_task(functor(), *jp, &tv[i]);
#else
        task *t = new_task(functor(), *jp);
#endif
        spawn(t);        
    }
    return 0;
}
