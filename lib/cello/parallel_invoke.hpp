#ifndef CELLO_PARALLEL_INVOKE_HPP
#define CELLO_PARALLEL_INVOKE_HPP
#include <cello/joiner.hpp>
#include <cello/scheduler.hpp>
#include <bsg_manycore.hpp>
#include <bsg_tile_config_vars.h>
namespace cello
{
/**
 * @brief Invoke two functions in parallel
 * @param f0 First function
 * @param f1 Second function
 */
template <typename F0, typename F1>
void parallel_invoke(F0 &&f0, F1 &&f1)
{
    using joiner = one_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
    task *t = new_task(std::forward<F0>(f0), *jp);
    spawn(t);
    f1();
    wait(jp);
    //delete t;
}

/**
 * @brief Invoke four functions in parallel
 * @param f0 First function
 * @param f1 Second function
 * @param f2 Third function
 * @param f3 Fourth function
 */
template <typename F0, typename F1, typename F2, typename F3>
void parallel_invoke(F0 &&f0, F1 &&f1, F2 &&f2, F3 &&f3)
{
    using joiner = three_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
    task *t0 = new_task(std::forward<F0>(f0), *jp);
    task *t1 = new_task(std::forward<F1>(f1), *jp);
    task *t2 = new_task(std::forward<F2>(f2), *jp);
    spawn(t0);
    spawn(t1);
    spawn(t2);
    f3();
    wait(jp);
    //delete t0;
    //delete t1;
    //delete t2;
}
}
#endif
