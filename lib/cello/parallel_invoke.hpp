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
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
    task *t = new_task(std::forward<F0>(f0), *jp);
    spawn(t);
    f1();
    wait(jp);
    delete t;
}
}
#endif
