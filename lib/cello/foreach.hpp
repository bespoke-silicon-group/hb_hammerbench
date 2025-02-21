#ifndef CELLO_FOREACH_HPP
#define CELLO_FOREACH_HPP
#include <cello/schedule.hpp>
#include <cello/parallel_foreach.hpp>

namespace cello {
/**
 * @brief foreach
 * @param begin
 * @param end
 * @param step
 * @param body
 */
template <cello::Schedule sched = parallel, typename Idx, typename Body>
void foreach(Idx begin, Idx end, Idx step, Body &&body)
{
    if (sched == parallel) {
        parallel_foreach(begin, end, step, std::forward(body));
    } else {
        for (Idx i = begin; i < end; i += step) {
            body(i);
        }
    }
}
}
#endif
