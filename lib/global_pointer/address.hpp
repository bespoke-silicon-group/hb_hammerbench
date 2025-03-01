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
    explicit address(address_ext ext, const void* raw) : ext_(ext), raw_(reinterpret_cast<uintptr_t>(raw)) {}

    /**
     * @brief constructor using default extended address
     */
    explicit address(const void * raw) : ext_(), raw_(reinterpret_cast<uintptr_t>(raw)) {}

    /**
     * @brief constructor using default extended address and null pointer
     */
    address() : ext_(), raw_(0) {}

    /**
     * @brief copy constructor
     */
    address(const address& other) : ext_(other.ext_), raw_(other.raw_) {}

    /**
     * @brief move constructor
     */
    address(address&& other) : ext_(other.ext_), raw_(other.raw_) {}

    /**
     * @brief copy assignment
     */
    address& operator=(const address& other) {
        ext_ = other.ext_;
        raw_ = other.raw_;
        return *this;
    }

    /**
     * @brief move assignment
     */
    address& operator=(address&& other) {
        ext_ = other.ext_;
        raw_ = other.raw_;
        return *this;
    }

    /**
     * @brief set the pod x of the address
     */
    address& set_pod_x(unsigned x) {
        ext().set_pod_x(x);
        return *this;
    }

    /**
     * @brief set the pod y of the address
     */
    address& set_pod_y(unsigned y) {
        ext().set_pod_y(y);
        return *this;
    }

    /**
     * @brief get the pod x of the address
     */
    unsigned pod_x() const {
        return ext().pod_x();
    }

    /**
     * @brief get the pod y of the address
     */
    unsigned pod_y() const {
        return ext().pod_y();
    }

    /**
     * @brief updates the value pointed to by the reference
     */
    template <typename T>
    void write(const T& other) {
        // problem, what if stack is in dram???
        // maybe we make this only valid for scalar types...
        register T wv = other;
        register T* ptr = reinterpret_cast<T*>(raw_);
        {
            pod_address_guard grd(ext_.pod_addr());
            *ptr = wv;
        }
    }

    /**
     * @brief reads the value pointed to by the reference
     */
    template <typename T>
    T read() const {
        register T rv;
        register T* ptr = reinterpret_cast<T*>(raw_);
        {
            pod_address_guard grd(ext_.pod_addr());
            rv = *ptr;
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
