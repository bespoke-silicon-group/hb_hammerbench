#include <util/lock.hpp>
#include <util/list.hpp>
#include <util/statics.hpp>
#include <cello/scheduler.hpp>
#include <cello/task_queue.hpp>
#include <cello/task.hpp>
#include <cello/thread_id.hpp>
#include <cello/pointer.hpp>
#include <global_pointer/global_pointer.hpp>
#include <bsg_manycore.h>
#include <bsg_manycore.hpp>
#include <bsg_tile_config_vars.h>
namespace cello
{

DMEM(int) scheduler_seed = 0;

// Not great in terms of randomness, but should be faster than rand()
inline int fast_random()
{
    scheduler_seed = ( 214013 * scheduler_seed + 2531011 );
    return ( scheduler_seed >> 16 ) & 0x7FFF;
}

using queue = util::lockable<task_queue, util::tile_lock>;

DMEM(queue) my_queue;

DMEM(queue *) my_queue_ptr = &my_queue;

global_pointer<queue> queue_of(int id)
{
    int pod, pod_x, pod_y, tile, tile_x, tile_y;
    pod    = id / my::num_tiles();
    tile   = id % my::num_tiles();
    pod_x  = pod % my::num_pods_x();
    pod_y  = pod / my::num_pods_x();
    tile_x = tile % my::num_tiles_x();
    tile_y = tile / my::num_tiles_x();
    queue *lcl = bsg_tile_group_remote_pointer<queue>(tile_x, tile_y, &my_queue);
    global_pointer<queue> glbl = global_pointer<queue>::onPodXY(pod_x, pod_y, lcl);
    return glbl;
}

/**
 * @brief initialize the scheduler
 */
void scheduler_initialize(config *cfg)
{
    scheduler_seed = my::tile_id();
    my_queue_ptr = bsg_tile_group_remote_pointer<queue>(my::tile_x(), my::tile_y(), &my_queue);
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
        int victim_id = fast_random() % my::num_tiles();
        auto victim_queue = queue_of(victim_id);
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
