#ifndef DATASTRUCTURE_FOREACH_HPP
#define DATASTRUCTURE_FOREACH_HPP
#ifndef HOST
#include <cello/cello.hpp>
#endif
namespace datastructure
{
/**
 * @brief foreach
 * @param begin
 * @param end
 * @param step
 * @param body
 */
template <typename Idx, typename Body>
void foreach(Idx begin, Idx end, Idx step, Body &&body)
{
#ifndef HOST
    parallel_foreach(begin, end, step, std::forward<Body>(body));
#else
    for (Idx i = begin; i < end; i += step) {
        body(i);
    }
#endif
}
/**
 * @brief foreach
 * @param begin
 * @param end
 * @param body
 */
template <typename Idx, typename Body>
void foreach(Idx begin, Idx end, Body &&body)
{
    foreach(begin, end, static_cast<Idx>(1), std::forward<Body>(body));
}
}
#endif
