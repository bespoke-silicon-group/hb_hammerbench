#ifndef CELLO_TASK_HPP
#define CELLO_TASK_HPP
#include <util/list.hpp>
#include <cello/joiner.hpp>
namespace cello
{

/**
 * @brief Task class
 */
class task
{
public:
    typedef joiner::child parent_ptr;
    void execute() {
        run();
        parent().join();
    }
    virtual void run() {}
    virtual ~task() {}
    virtual size_t size() const { return sizeof(*this); }
    FIELD(util::list, ready);
    FIELD(cello::joiner::child, parent);
};

/**
 * @brief
 */
}
#endif
