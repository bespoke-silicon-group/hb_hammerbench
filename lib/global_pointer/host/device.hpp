#ifndef GLOBAL_POINTER_HOST_DEVICE_HPP
#define GLOBAL_POINTER_HOST_DEVICE_HPP
#ifndef HOST
#error "This file should only be included in host code"
#endif
#include <bsg_manycore_cuda.h>
namespace bsg_global_pointer
{

/**
 * singleton device pointer
 */
extern hb_mc_device_t * the_device;

}
#endif
