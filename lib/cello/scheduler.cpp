#include <cello/scheduler.hpp>
#include <bsg_manycore.h>
namespace cello
{
/**
 * detach a task from the current thread of execution
 */
void spawn(task * t)
{
    //    bsg_print_hexadecimal((unsigned)t);
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
