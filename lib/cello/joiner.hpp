#ifndef CELLO_JOINER_HPP
#define CELLO_JOINER_HPP
#include <util/class_field.hpp>
namespace cello
{

/**
 * @brief Joiner class
 */
class joiner
{
public:
    typedef joiner* joiner_ptr;

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
    joiner() : ready_(0) {}

    /**
     * signal that child has completed
     */
    void increment_ready_count() {
        ready() = 1;
    }

    /**
     * join has completed
     */
    bool joined() const { return ready(); }

    FIELD(int, ready);
};

}
#endif
