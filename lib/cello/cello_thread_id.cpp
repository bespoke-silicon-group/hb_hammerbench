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
}
/**
 * @brief setup the id variables
 */
void set_id_vars(config *cfg) {
    using namespace cello;
    my::__bsg_pod_x = cfg->pod_x();
    my::__bsg_pod_y = cfg->pod_y();
    my::__bsg_pod_id = my::pod_x() + my::num_pods_x() * my::pod_y();
}
}
