#ifndef GLOBAL_POINTER_ADDRESS_HPP
#define GLOBAL_POINTER_ADDRESS_HPP
#include <stdint.h>
#include <bsg_manycore.h>
#include <util/class_field.hpp>
#include <global_pointer/address_ext.hpp>

namespace bsg_global_pointer
{

/**
 * @brief address
 */
class address
{
public:
    /**
     * @brief base constructor
     */
    address(address_ext ext, void* raw) : ext_(ext), raw_(reinterpret_cast<uintptr_t>(raw)) {}

    /**
     * @brief constructor using default extended address
     */
    address(uintptr_t raw) : ext_(), raw_(raw) {}

    /**
     * @brief constructor using default extended address and null pointer
     */
    address() : ext_(), raw_(0) {}


    /**
     * @brief updates the value pointed to by the reference
     */
    template <typename T>
    void write(const T& other) {
        // problem, what if stack is in dram???
        // maybe we make this only valid for scalar types...
        register T wv = other;
        {
            pod_address_guard grd(ext_.pod_addr());
            *reinterpret_cast<T*>(raw_) = wv;
        }
    }

    /**
     * @brief reads the value pointed to by the reference
     */
    template <typename T>
    T read() const {
        register T rv;
        {
            pod_address_guard grd(ext_.pod_addr());
            rv = *reinterpret_cast<T*>(raw_);
        }
        return rv;
    }

    /**
     * @brief add operator
     */
    address operator+(uintptr_t off) const {
        return address(ext_, reinterpret_cast<void*>(raw_ + off));
    }

    FIELD(address_ext ,ext); //!< extended address information
    FIELD(uintptr_t   ,raw); //!< the raw pointer
};

}
#endif
