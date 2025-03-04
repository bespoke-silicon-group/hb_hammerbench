#ifndef CELLO_SCHEDULER_HPP
#define CELLO_SCHEDULER_HPP
#include <cello/task.hpp>
#include <cello/joiner.hpp>
#include <cello/config.hpp>
#include <bsg_manycore.h>
namespace cello {
/**
 * @brief initialize the scheduler
 */
void scheduler_initialize(config *cfg);

/**
 * detach a task from the current thread of execution
 */
void spawn(task *t);

/**
 * delegate a task to a pod
 */
void delegate(int pod_x, int pod_y, task *t);

/**
 * delegate a task to a pod
 */
void delegate(int pod, task *t);

/**
 * wait for a joiner to be ready
 */
void wait(joiner_base *j);

/**
 * schedule work on this thread
 */
void schedule();

/**
 * schedule work until condition is true
 */
template <typename Condition>
void scheduler_loop(Condition &&condition) {
    while (!condition()) {
        schedule();
    }
}
}

#endif
