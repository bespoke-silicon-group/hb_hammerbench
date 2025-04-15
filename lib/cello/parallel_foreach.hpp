#ifndef CELLO_PARALLEL_FOR_HPP
#define CELLO_PARALLEL_FOR_HPP
#include <cello/loop_info.hpp>
#include <cello/parallel_invoke.hpp>
#include <utility>
namespace cello
{

template <typename Idx, typename Body>
void parallel_foreach(loop_info<Idx> &info, Body &&body);

/**
 * @brief a child branch functor
 */
template <typename Idx, typename Body>
struct parallel_foreach_child {
    parallel_foreach_child(loop_info<Idx> &&info, Body &&body)
        : info_(std::move(info)), body_(body){}
    FIELD(loop_info<Idx>, info);
    typename std::remove_reference<Body>::type body_;
    void operator()() {
        cello::parallel_foreach<Idx, Body>(info(), Body{body_});
    }
};

/**
 * @brief parallel_foreac
 * @param info
 * @param bodyh
 */
template <typename Idx, typename Body>
void parallel_foreach(loop_info<Idx> &info, Body &&body)
{
#ifdef CELLO_PARALLEL_FOREACH_RECURSE
    if (info.leafs() <= 1) {
        for (Idx i = info.start(); i < info.stop(); i += info.step()) {
            body(i);
        }
    } else {
        parallel_invoke
            (parallel_foreach_child<Idx,Body>(info.lower(), std::forward<Body>(body)),
             parallel_foreach_child<Idx,Body>(info.upper(), std::forward<Body>(body)));
    }
#else
    using joiner = n_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(__bsg_x, __bsg_y, &j);
    Idx h = info.height();
    task **tpv = static_cast<task**>(cello::allocate(sizeof(task*)*h));
    size_t k = 0;
    while (info.leafs() > 1) {
        task *tp = new_task(parallel_foreach_child<Idx,Body>(info.lower(), std::forward<Body>(body)),
                            *jp);
        spawn(tp);
        tpv[k++] = tp;
        info = info.upper();
    }
    for (Idx i = info.start(); i < info.stop(); i+= info.step()) {
        body(i);
    }
    wait(jp);
    for (k = 0; k < h; k++) {
        delete tpv[k];
    }
    cello::deallocate(tpv, sizeof(task*)*h);
#endif
}

/**
 * @brief parallel_foreac
 * @param begin
 * @param end
 * @param body
 */
template <typename Idx, typename Body>
void parallel_foreach(Idx begin, Idx end, Body &&body)
{
    loop_info<Idx> info(begin, end, 1);
    parallel_foreach(info, std::forward<Body>(body));
}


/**
 * @brief parallel_foreac
 * @param begin
 * @param end
 * @param step
 * @param body
 */
template <typename Idx, typename Body>
void parallel_foreach(Idx begin, Idx end, Idx step, Body &&body)
{
    loop_info<Idx> info(begin, end, step);
    parallel_foreach(info, std::forward<Body>(body));
}

}
#endif
