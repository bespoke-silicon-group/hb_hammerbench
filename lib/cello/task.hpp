#ifndef CELLO_TASK_HPP
#define CELLO_TASK_HPP
#include <util/list.hpp>
#include <util/class_field.hpp>
#include <cello/joiner.hpp>
#include <cello/allocator.hpp>
#include <cello/pointer.hpp>
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
    /**
     * @brief constructor
     */
    task() {}

    /**
     * @brief destructor
     */
    virtual ~task() {}

    /**
     * @brief runs the task and joins with parent
     */
    virtual void execute() {}

    /**
     * @brief returns the size of the task
     */
    virtual size_t size() const {
        return sizeof(*this);
    }

    FIELD(util::list_item, queued);
};

/**
 * @brief Functor task
 */
template <typename F, typename Joiner>
class functor_task : public task {
public:
    /**
     * @brief constructor
     */
    functor_task(F && f, Joiner & p)
        : task()
        , parent_(p.make_child())
        , func(std::move(f)) {
    }

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

    /**
     * @brief returns the size of the task
     */
    virtual size_t size() const {
        return sizeof(*this);
    }    

    void *operator new(size_t size) {
        return allocate(sizeof(functor_task<F, Joiner>));
    }

    void operator delete(void *p) {
        deallocate(p, sizeof(functor_task<F, Joiner>));
    }

    typename std::remove_reference<F>::type func; //!< function lambda, no reference

    FIELD(typename Joiner::child, parent);
};

template <typename F, typename Joiner>
inline task * new_task(F && f, Joiner &j) {
    return new functor_task<F, Joiner>(std::forward<F>(f), j);
}
}

template <>
class bsg_global_pointer::reference<cello::task> {
    BSG_GLOBAL_POINTER_REFERENCE_TRIVIAL(cello::task);
    BSG_GLOBAL_POINTER_REFERENCE_METHOD(cello::task, execute);
    BSG_GLOBAL_POINTER_REFERENCE_FUNCTION(cello::task, size, size_t);
};
#endif
