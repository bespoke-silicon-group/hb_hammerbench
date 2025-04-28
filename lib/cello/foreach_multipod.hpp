#ifndef CELLO_FOREACH_MULTIPOD_HPP
#define CELLO_FOREACH_MULTIPOD_HPP
#include <cello/foreach.hpp>
#include <cello/delegate.hpp>
namespace cello
{
template <typename cello::Schedule sched = cello::parallel,
          typename Idx,
          typename F>
void foreach_multipod(Idx begin, Idx end, Idx stride, F && f) {
    using namespace cello;
    using joiner = n_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(my::tile_x(), my::tile_y(), &j);
    on_every_pod(jp, [begin, end, stride, f](){
        size_t start = stride * my::pod_id();
        size_t step = stride * my::num_pods();
        cello::foreach<sched>
            (start, end, step, [=](size_t i){
                size_t start = i;
                size_t stop = i + stride;
                if (stop > end) {
                    stop = end;
                }
                for (size_t j = start; j < stop; j++) {
                    f(j);
                }
            });
    });
    wait(&j);
}

template <typename cello::Schedule sched = cello::parallel,
          typename Idx,
          typename F>
void foreach_block_multipod(Idx begin, Idx end, Idx stride, F && f) {
    using namespace cello;
    using joiner = n_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(my::tile_x(), my::tile_y(), &j);
    on_every_pod(jp, [begin, end, stride, f](){
        size_t start = stride * my::pod_id();
        size_t step = stride * my::num_pods();
        cello::foreach<sched>
            (start, end, step, [=](size_t i){
                size_t start = i;
                size_t stop = i + stride;
                if (stop > end) {
                    stop = end;
                }
                f(start, stop);
            });
    });
    wait(&j);
}
}
#endif
