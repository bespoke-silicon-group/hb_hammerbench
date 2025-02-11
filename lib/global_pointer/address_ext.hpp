#ifndef GLOBAL_POINTER_ADDRESS_EXT_HPP
#define GLOBAL_POINTER_ADDRESS_EXT_HPP
#include <util/class_field.hpp>
#include <global_pointer/pod_address.hpp>
namespace bsg_global_pointer
{
/**
 * extended address information
 */
class address_ext
{
public:
    address_ext() : pod_addr_(0) {}
    FIELD(pod_address, pod_addr);
};
}
#endif
