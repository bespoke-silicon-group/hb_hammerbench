#ifndef CELLO_JOINER_HPP
#define CELLO_JOINER_HPP
#include <util/class_field.hpp>
#include <bsg_manycore.h>
#include <bsg_tile_config_vars.h>
namespace cello
{

/**
 * @brief joiner base class
 */
class joiner_base
{
public:
    virtual bool joined() const = 0;
};

/**
 * @brief Joiner class
 */
class one_child_joiner : public joiner_base
{
public:
    typedef one_child_joiner* joiner_ptr;

    /**
     * @brief a child of this joiner
     */
    class child {
    public:
        child(joiner_ptr p) : parent_(p) {}
        void join() {
            parent_->increment_ready_count();
        }
        FIELD(joiner_ptr, parent);        
    };

    /**
     * currently only one child supported
     */
    one_child_joiner() : ready_(0) {}

    /**
     * signal that child has completed
     */
    void increment_ready_count() {
        ready() = 1;
    }

    /**
     * join has completed
     */
    bool joined() const override { return ready(); }

    /**
     * returns a created child
     */
    child make_child() {
        return child(this);
    }

    FIELD(int, ready);
};

/**
 */
}
#endif
