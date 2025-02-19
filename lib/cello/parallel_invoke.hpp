#ifndef CELLO_PARALLEL_INVOKE_HPP
#define CELLO_PARALLEL_INVOKE_HPP
#include <cello/joiner.hpp>
#include <cello/scheduler.hpp>
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
    task *t = new_task(f0, j);
    spawn(t);
    f1();
    wait(&j);
    delete t;
}
}
#endif
