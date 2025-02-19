#ifndef CELLO_INVOKE_HPP
#define CELLO_INVOKE_HPP
#include <utility>
#include <cello/schedule.hpp>
#include <cello/parallel_invoke.hpp>
namespace cello
{

/**
 * @brief execute two functions
 */
template <cello::Schedule sched = parallel, typename F0, typename F1>
void invoke(F0 && f0, F1 && f1) {
    if (sched == parallel) {
        parallel_invoke
            (std::forward(f0),
             std::forward(f1));
    } else {
        f0();
        f1();
    }
}


}
#endif
