#ifndef CELLO_TASK_QUEUE_HPP
#define CELLO_TASK_QUEUE_HPP
#include <cello/task.hpp>
#include <cello/stats.hpp>
#include <cello/pointer.hpp>
#include <util/list.hpp>
#include <util/class_field.hpp>
#include <util/lock.hpp>
namespace cello
{

/**
 * @brief task_queue
 */
class task_queue
{
public:

    /**
     * @brief push a task onto the queue as the owner
     */
    void owner_push(task * t) {
        task_list().push_front(&t->queued());
    }

    /**
     * @brief pop a task from the queue as the owner
     */
    task * owner_pop() {
        if (task_list().empty()) {
            return nullptr;
        }
        util::list_item*l = task_list().pop_front();
        return container_of(l, task, queued_);
    }

    /**
     * @brief  pop a task from the queue as a thief
     */
    task * thief_pop() {
        if (task_list().empty()) {
            return nullptr;
        }
        util::list_item*l = task_list().pop_back();
        return container_of(l, task, queued_);
    }

    /**
     * @brief check if the queue is empty
     */
    bool empty() const {
        return task_list().empty();
    }

    /**
     * @brief check if the queue is empty - cannot be compiled out
     */
    bool empty_volatile() const {
        return task_list().empty_volatile();
    }

    FIELD(util::list, task_list);
};
}

template <typename Lock>
class util::lockable<cello::task_queue, Lock>
{
public:
    UTIL_LOCKABLE_INTERNAL(cello::task_queue, Lock);
    //UTIL_LOCKABLE_METHOD(cello::task_queue, Lock, owner_push);
    void owner_push(cello::task *t) {
        while (!lock_.try_acquire()) {
            CELLO_STAT_ADD(cello_owner_lock_acquire_fail);
        }
        data_.owner_push(t);
        lock_.release();
    }
    //UTIL_LOCKABLE_FUNCTION(cello::task_queue, Lock, cello::task*, owner_pop);
    cello::task *owner_pop() {
        if (data_.empty_volatile())
            return nullptr;

        cello::task* r;
        while (!lock_.try_acquire()) {
            CELLO_STAT_ADD(cello_owner_lock_acquire_fail);
        }
        r = data_.owner_pop();
        lock_.release();
        return r;
    }
    UTIL_LOCKABLE_FUNCTION_CAN_FAIL(cello::task_queue, Lock, cello::task*, nullptr, thief_pop);
    UTIL_LOCKABLE_FUNCTION_CONST(cello::task_queue, Lock, bool, empty);
    UTIL_LOCKABLE_FUNCTION_CONST(cello::task_queue, Lock, bool, empty_volatile);
};

template <typename Lock>
class bsg_global_pointer::reference<util::lockable<cello::task_queue, Lock>>
{
public:
    using type = util::lockable<cello::task_queue, Lock>;
    BSG_GLOBAL_POINTER_REFERENCE_TRIVIAL(type);
    BSG_GLOBAL_POINTER_REFERENCE_METHOD(type, owner_push);
    BSG_GLOBAL_POINTER_REFERENCE_FUNCTION(type, owner_pop, cello::task*);
    cello::global_pointer<cello::task> thief_pop() {
        type*p = reinterpret_cast<type*>(addr().raw());
        cello::task *r;
        {
            pod_address_guard grd(addr().ext().pod_addr());
            r = p->thief_pop();
        }
        return cello::global_pointer<cello::task>::withExtAddr(addr().ext(), r);
    }
    //BSG_GLOBAL_POINTER_REFERENCE_FUNCTION(type, thief_pop, cello::task*);
    BSG_GLOBAL_POINTER_REFERENCE_FUNCTION(type, empty, bool);
};

#endif
