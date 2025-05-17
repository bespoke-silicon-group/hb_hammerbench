#ifndef CELLO_DELEGATE_QUEUE_HPP
#define CELLO_DELEGATE_QUEUE_HPP
#include <cello/task.hpp>
#include <cello/pointer.hpp>
#include <cello/thread_id.hpp>
#include <util/list.hpp>
#include <util/class_field.hpp>
#include <util/lock.hpp>
namespace cello
{
/**
 * @brief delegate queue
 * queue of delegate tasks
 */
class delegate_queue
{
public:
    /**
     * @brief push a task onto the queue
     */
    void delegater_push(task * t) {
        task_list().push_back(&t->queued_);
    }

    /**
     * @brief pop a task from the queue
     */
    task *owner_pop() {
        if (task_list().empty()) {
            return nullptr;
        }
        util::list_item*l = task_list().pop_front();
        return container_of(l, task, queued_);
    }

    /**
     * @brief check if the queue is empty
     */
    bool empty() const {
        return task_list().empty();
    }

    FIELD(util::list, task_list);
};
}

template <typename Lock>
class util::lockable<cello::delegate_queue, Lock>
{
public:
    UTIL_LOCKABLE_INTERNAL(cello::delegate_queue, Lock);
    UTIL_LOCKABLE_METHOD(cello::delegate_queue, Lock, delegater_push);
    UTIL_LOCKABLE_FUNCTION(cello::delegate_queue, Lock, cello::task *, owner_pop);
    UTIL_LOCKABLE_FUNCTION(cello::delegate_queue, Lock, bool, empty);
};

template <typename Lock>
class bsg_global_pointer::reference<util::lockable<cello::delegate_queue, Lock>>
{
    using type = util::lockable<cello::delegate_queue, Lock>;
    BSG_GLOBAL_POINTER_REFERENCE_TRIVIAL(type);

    /**
     * @brief push a task onto the queue
     * @param tp task pointer
     *
     * assumption: tp points to memory in the calling core's native memory space
     */
    void delegater_push(const bsg_global_pointer::pointer<cello::task> &tp) {
        if (tp.pod_x() == pod_x() && tp.pod_y() == pod_y()) {
            register cello::task *native_ptr = reinterpret_cast<cello::task*>(tp.ref().addr().raw());
            register util::lockable<cello::delegate_queue, Lock> *queue = reinterpret_cast<util::lockable<cello::delegate_queue, Lock>*>(addr().raw());
            {
                pod_address_guard grd(addr().ext().pod_addr());
                queue->delegater_push(native_ptr);
            }
        } else {
            //1. allocate memory with native affinity to this queue
            //2. copy the task to the new memory
            //3. push the new task to the queue
            register size_t size = tp->size();
            register cello::task* dst_task;
            register util::lockable<cello::delegate_queue, Lock> *queue = reinterpret_cast<util::lockable<cello::delegate_queue, Lock>*>(addr().raw());
            {
                pod_address_guard grd(addr().ext().pod_addr());
                dst_task = reinterpret_cast<cello::task*>(cello::allocate(size));
                bsg_global_pointer::memcpy
                    (reinterpret_cast<char*>(dst_task)
                     , bsg_global_pointer::pointer_cast<char>(tp)
                     , size
                     );
                queue->delegater_push(dst_task);
            }
            // delete the original task, since it has been moved
            cello::task *native_ptr = reinterpret_cast<cello::task*>(tp.ref().addr().raw());
            //delete native_ptr;
        }
    }
    
    BSG_GLOBAL_POINTER_REFERENCE_FUNCTION(type, owner_pop, cello::task *);
};
#endif // CELLO_DELEGATE_QUEUE_HPP
