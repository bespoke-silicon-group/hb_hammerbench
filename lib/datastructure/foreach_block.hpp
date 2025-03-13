#ifndef DATASTRUCTURE_FOREACH_BLOCK_HPP
#define DATASTRUCTURE_FOREACH_BLOCK_HPP
#include <datastructure/foreach.hpp>
namespace datastructure
{
template <typename Idx, typename Body>
void foreach_block(Idx start, Idx stop, Idx block_size, Body &&body)
{
    datastructure::foreach<Idx>(start, stop, block_size, [body, block_size, stop] (Idx i) {
        Idx j = i + block_size;
        if (j > stop) {
            j = stop;
        }
        body(i, j);
    });
}

template <int BLOCK_SIZE, typename Idx, typename Body>
void foreach_block(Idx start, Idx stop, Body &&body)
{
    datastructure::foreach<Idx>(start, stop, BLOCK_SIZE, [body, stop](Idx i) {
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
