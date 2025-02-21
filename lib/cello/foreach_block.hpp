#ifndef CELLO_FOREACH_BLOCK_HPP
#define CELLO_FOREACH_BLOCK_HPP
#include <cello/foreach.hpp>
#include <cello/schedule.hpp>
namespace cello
{
template <Schedule sched = cello::serial, typename Idx, typename Body>
void foreach_block(Idx start, Idx stop, Idx block_size, Body &&body)
{
    cello::foreach<sched, Idx>(start, stop, block_size, [body, block_size, stop] (Idx i) {
        Idx j = i + block_size;
        if (j > stop) {
            j = stop;
        }
        body(i, j);
    });
}

template <Schedule sched = cello::serial, int BLOCK_SIZE, typename Idx, typename Body>
void foreach_block(Idx start, Idx stop, Body &&body)
{
    cello::foreach<sched, Idx>(start, stop, BLOCK_SIZE, [body, stop](Idx i) {
        constexpr Idx block_size = BLOCK_SIZE;
        Idx j = i + block_size;
        if (j > stop) {
            j = stop;
        }
        body(i, j);
    });
}
}
#endif
