#ifndef CELLO_SCHEDULER_HPP
#define CELLO_SCHEDULER_HPP
#include <cello/task.hpp>
#include <cello/joiner.hpp>
namespace cello {

/**
 * detach a task from the current thread of execution
 */
void spawn(task *t);

/**
 * wait for a joiner to be ready
 */
void wait(joiner *j);

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
