#include <cello/thread_id.hpp>
#include <util/statics.hpp>
namespace cello
{
namespace my
{
/**
 * @brief get the pod x of this pod
 */
DMEM(int) __bsg_pod_x;
/**
 * @brief get the pod y of this pod
 */
DMEM(int) __bsg_pod_y;
/**
 * @brief get the pod id of this pod
 */
DMEM(int) __bsg_pod_id;
/**
 * @brief get the absolute id of this tile
 */
DMEM(int) __bsg_absolute_id;
}
/**
 * @brief setup the id variables
 */
void set_id_vars(config *cfg) {
    using namespace cello;
    my::__bsg_pod_x = cfg->pod_x();
    my::__bsg_pod_y = cfg->pod_y();
    my::__bsg_pod_id = my::pod_x() + my::num_pods_x() * my::pod_y();
    my::__bsg_absolute_id = my::pod_id() * my::num_tiles() + my::tile_id();
}

/**
 * @brief decode the id
 */
void thread_id_decode(thread_id_decoded *decode, int id)
{
    decode->pod    = id / my::num_tiles();
    decode->tile   = id % my::num_tiles();
    decode->pod_x  = decode->pod % my::num_pods_x();
    decode->pod_y  = decode->pod / my::num_pods_x();
    decode->tile_x = decode->tile % my::num_tiles_x();
    decode->tile_y = decode->tile / my::num_tiles_x();
}
}
