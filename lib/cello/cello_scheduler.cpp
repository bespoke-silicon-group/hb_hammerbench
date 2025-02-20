#include <util/lock.hpp>
#include <util/list.hpp>
#include <cello/scheduler.hpp>
#include <cello/task_queue.hpp>
#include <cello/task.hpp>
#include <bsg_manycore.h>
#include <bsg_manycore.hpp>
#include <bsg_tile_config_vars.h>
namespace cello
{

__attribute__((section(".dmem")))
int scheduler_seed = 0;

// Not great in terms of randomness, but should be faster than rand()
inline int fast_random()
{
    scheduler_seed = ( 214013 * scheduler_seed + 2531011 );
    return ( scheduler_seed >> 16 ) & 0x7FFF;
}

using queue = util::lockable<task_queue, util::tile_lock>;

__attribute__((section(".dmem")))
queue my_queue;

__attribute__((section(".dmem")))
queue *my_queue_ptr = &my_queue;

queue *queue_of(int id)
{
    return bsg_tile_group_remote_pointer<queue>(id % bsg_tiles_X, id / bsg_tiles_X, &my_queue);
}

/**
 * @brief initialize the scheduler
 */
void scheduler_initialize(config *cfg)
{
    scheduler_seed = __bsg_id;
    my_queue_ptr = bsg_tile_group_remote_pointer<queue>(__bsg_x, __bsg_y, &my_queue);
    new (my_queue_ptr) queue;
}

/**
 * detach a task from the current thread of execution
 */
void spawn(task * t)
{
    my_queue_ptr->owner_push(t);
}


/**
 * schedule work on this thread
 */
void schedule()
{
    task * t = my_queue_ptr->owner_pop();
    if (t) {
        t->execute();
    } else {
        int victim_id = fast_random() % (bsg_tiles_X * bsg_tiles_Y);
        auto *victim_queue = queue_of(victim_id);
        task * t = victim_queue->thief_pop();
        if (t) {
            //bsg_print_int(1000000 + __bsg_id * 1000 + victim_id);
            t->execute();
        }
    }
}

/**
 * wait for a joiner to be ready
 */
void wait(joiner_base * j)
{
    scheduler_loop([j](){ return j->joined(); });
}
}
