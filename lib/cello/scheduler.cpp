#include <cello/scheduler.hpp>
namespace cello
{
/**
 * detach a task from the current thread of execution
 */
void spawn(task * t)
{
    t->execute();
}


/**
 * schedule work on this thread
 */
void schedule()
{
    return;;
}

/**
 * wait for a joiner to be ready
 */
void wait(joiner * j)
{
    scheduler_loop([j](){ return j->ready(); });
}
}
