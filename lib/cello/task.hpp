#ifndef CELLO_TASK_HPP
#define CELLO_TASK_HPP
#include <util/list.hpp>
#include <util/class_field.hpp>
#include <cello/joiner.hpp>
#include <cello/allocator.hpp>
#include <new>
#include <utility>
namespace cello
{

/**
 * @brief Task class
 */
class task
{
public:
    typedef joiner::child parent_ptr;

    /**
     * @brief constructor
     */
    task(parent_ptr _parent) : parent_(_parent) {}

    /**
     * @brief destructor
     */
    virtual ~task() {}

    /**
     * @brief runs the task and joins with parent
     */
    virtual void execute() {
        parent().join();
    }

    /**
     * @brief returns the size of this task
     */
    virtual size_t size() const { return sizeof(*this); }

    FIELD(util::list, ready);
    FIELD(cello::joiner::child, parent);
};

/**
 * @brief Functor task
 */
template <typename F>
class functor_task : public task{
public:
    /**
     * @brief constructor
     */
    functor_task(F && f, parent_ptr p) : task(p), func(f) {}

    /**
     * destructor
     */
    virtual ~functor_task() {}
    
    /**
     * run the task
     */
    virtual void execute() {
        func();
        parent().join();
    }

    void *operator new(size_t size) {
        return allocate(sizeof(functor_task<F>));
    }

    void operator delete(void *p) {
        deallocate(p, sizeof(functor_task<F>));
    }

    F func;
};

template <typename F>
inline task * new_task(F && f, joiner &j) {
    return new functor_task<F>(f, j.make_child());
}

}
#endif
