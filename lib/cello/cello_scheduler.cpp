#include <util/lock.hpp>
#include <util/list.hpp>
#include <cello/scheduler.hpp>
#include <cello/task_queue.hpp>
#include <cello/task.hpp>
#include <bsg_manycore.h>

namespace cello
{

__attribute__((section(".dmem")))
util::lockable<task_queue, util::tile_lock> my_queue;


/**
 * @brief initialize the scheduler
 */
void scheduler_initialize(config *cfg)
{
    new (&my_queue) util::lockable<task_queue, util::tile_lock>();
}

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
