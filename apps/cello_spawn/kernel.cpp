#include <cello/cello.hpp>
#include <cello/task_queue.hpp>
#include <util/lock.hpp>
#include <array>

namespace cello
{
extern util::lockable<task_queue, util::tile_lock> *my_tasks_ptr;
}
struct functor {
    void operator()() {}
    std::array<unsigned, TASK_SIZE/sizeof(unsigned)> payload = {};
};

int cello_main(int argc, char *argv[])
{
    using namespace cello;
#ifdef ONE_CHILD_JOINER
    using joiner = one_child_joiner;
#else
    using joiner = n_child_joiner;
#endif
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
    using ftask = functor_task<functor, joiner>;
    //bsg_print_int(sizeof(ftask));
#ifdef ALLOCATE_TASK_IN_DMEM
    char buf[sizeof(ftask)];
    ftask *tv = reinterpret_cast<ftask*>(buf);
    tv = bsg_tile_group_remote_pointer<ftask>(__bsg_x, __bsg_y, tv);
#endif
    for (int i = 0; i < N; i++) {
#ifdef ALLOCATE_TASK_IN_DMEM
        task *t = new_task(functor(), *jp, tv);
#else
        task *t = new_task(functor(), *jp);
#endif
#ifndef NO_SPAWN
        spawn(t);
#else
        bsg_fence();
#endif
#ifdef ALLOCATE_TASK_IN_DMEM
        my_tasks_ptr->clear_unsafe();
#endif
    }
    return 0;
}
