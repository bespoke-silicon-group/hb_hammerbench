#ifndef CELLO_PARALLEL_FOREACH_BLOCK_HPP
#define CELLO_PARALLEL_FOREACH_BLOCK_HPP
#include <cello/foreach_block.hpp>
namespace cello
{
template <typename Idx, typename Body>
void parallel_foreach_block(Idx start, Idx stop, Idx block_size, Body &&body)
{
    cello::foreach_block<cello::parallel, Idx>(start, stop, block_size, body);
}

template <int BLOCK_SIZE, typename Idx, typename Body>
void parallel_foreach_block(Idx start, Idx stop, Body &&body)
{
    cello::parallel_foreach_block<cello::parallel, BLOCK_SIZE, Idx>(start, stop, body);
}
}
#endif
