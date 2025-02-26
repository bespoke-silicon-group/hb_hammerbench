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

void decode_id(int id, int &pod, int &pod_x, int &pod_y, int &tile, int &tile_x, int &tile_y)
{
    pod    = id / my::num_tiles();
    tile   = id % my::num_tiles();
    pod_x  = pod % my::num_pods_x();
    pod_y  = pod / my::num_pods_x();
    tile_x = tile % my::num_tiles_x();
    tile_y = tile / my::num_tiles_x();    
}

global_pointer<queue> queue_of(int id)
{
    int pod, pod_x, pod_y, tile, tile_x, tile_y;
    decode_id(id, pod, pod_x, pod_y, tile, tile_x, tile_y);

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

void execute_task(int victim_id, task * t)
{
    t->execute();
}

void execute_task(int victim_id, global_pointer<task> t)
{
    int pod, pod_x, pod_y, tile, tile_x, tile_y;
    decode_id(victim_id, pod, pod_x, pod_y, tile, tile_x, tile_y);
    if (pod != my::pod_id()) {
        task *lcl = reinterpret_cast<task*>(allocate(t->size()));
        bsg_global_pointer::memcpy
            (reinterpret_cast<char*>(lcl)
             ,bsg_global_pointer::pointer_cast<char, task>(t)
             ,t->size()
             );
        lcl->execute();
        deallocate(lcl, t->size());
    }else {
        t->execute();
    }
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
        int victim_id = fast_random() % my::num_tiles_total();
        auto victim_queue = queue_of(victim_id);
        auto t = victim_queue->thief_pop();
        if (!is_null(t)) {
            //bsg_print_int(1000000 + victim_id*1000 + my::id());            
            execute_task(victim_id, t);
        }
    }
}

/**
 * wait for a joiner to be ready
 */
void wait(joiner_base * j)
{
    //bsg_print_hexadecimal((unsigned)j);
    scheduler_loop([j](){
        return j->joined();
    });
    //bsg_print_hexadecimal((unsigned)j | 0x10000000);    
}
}
