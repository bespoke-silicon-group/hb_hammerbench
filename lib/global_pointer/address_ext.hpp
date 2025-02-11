#ifndef GLOBAL_POINTER_ADDRESS_EXT_HPP
#define GLOBAL_POINTER_ADDRESS_EXT_HPP
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
    unsigned pod_addr_ : pod_address::bits;
};
}
#endif
