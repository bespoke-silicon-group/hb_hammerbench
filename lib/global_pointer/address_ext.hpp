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
    address_ext(const pod_address &pod) : pod_addr_(pod) {}
    address_ext() : pod_addr_() {}
    address_ext &set_pod_x(unsigned x) {
        pod_addr().set_pod_x(x);
        return *this;
    }
    address_ext &set_pod_y(unsigned y) {
        pod_addr().set_pod_y(y);
        return *this;
    }
    unsigned pod_x() const {
        return pod_addr().pod_x();
    }
    unsigned pod_y() const {
        return pod_addr().pod_y();
    }
    bool operator==(const address_ext &other) const {
        return pod_addr() == other.pod_addr();
    }
    FIELD(pod_address, pod_addr);
};
}
#endif
