#ifndef CELLO_DELEGATE_HPP
#define CELLO_DELEGATE_HPP
#include <cello/task.hpp>
#include <cello/scheduler.hpp>
namespace cello
{

/**
 * @brief call function on another core
 * @param an absolute tile id
 * @param a pointer to a joiner: this must be a shared pointer like dram or tile-group
 * @param a function to call
 *
 * to synchronized with the called function, wait() should be called on the joiner
 */
template <typename F, typename Joiner>
void on_tile(int absolute_tile_id, Joiner *jp, F && f)
{
    task *t = new_task(std::forward<F>(f), *jp);
    delegate(absolute_tile_id, t);
}

/**
 * @brief call function on another core
 * @param an absolute tile id
 * @param a function to call
 *
 * returns when the call has completed
 */
template <typename F>
void on_tile(int absolute_tile_id, F && f)
{
    using joiner = one_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(my::tile_x(), my::tile_y(), &j);
    on_tile(absolute_tile_id, jp, std::forward<F>(f));
    wait(jp);
}

/**
 * @brief call function on another pod
 * @param a pod id
 * @param a pointer to a joiner: this must be a shared pointer like dram or tile-group
 * @param a function call
 *
 * does not block
 */
template <typename F, typename Joiner>
void on_pod(int pod_id, Joiner *jp, F && f)
{
    task *t = new_task(std::forward<F>(f), *jp);
    delegate_pod(pod_id, t);
}

/**
 * @brief call function on another pod
 * @param a pod id
 * @param a function call
 *
 * blocks until call completes
 */
template <typename F>
void on_pod(int pod_id, F && f)
{
    using joiner = one_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(my::tile_x(), my::tile_y(), &j);
    on_pod(pod_id, jp, std::forward<F>(f));
    wait(jp);
}

/**
 * @brief call a function on every pod
 * @param pointer to a joiner: this must be a shared pointer like dram or tile-group
 * @param a function call
 *
 * does not block
 */
template <typename F>
void on_every_pod(n_child_joiner *jp, F &&f)
{    
    cello::foreach<cello::parallel>(0, my::num_pods(), [jp, f](int pod){
        on_pod(pod, jp, f);
    });
}

/**
 * @brief call function on every pod
 * @param a pod id
 * @param a function call
 *
 * blocks until all calls complete
 */
template <typename F>
void on_every_pod(F && f) {
    using joiner = n_child_joiner;
    joiner j;
    joiner *jp = bsg_tile_group_remote_pointer<joiner>(my::tile_x(), my::tile_y(), &j);
    on_every_pod(jp, std::forward<F>(f));
    wait(jp);
}

}
#endif
