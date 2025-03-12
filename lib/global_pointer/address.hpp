#ifndef GLOBAL_POINTER_ADDRESS_HPP
#define GLOBAL_POINTER_ADDRESS_HPP
#include <bsg_manycore.h>
#include <util/class_field.hpp>
#include <global_pointer/intptr.hpp>
#include <global_pointer/address_ext.hpp>

namespace bsg_global_pointer
{

/**
 * @brief address
 */
class address
{
public:
#ifndef HOST
    /**
     * @brief base constructor
     */
    explicit address(address_ext ext, const void* raw) : ext_(ext), raw_(reinterpret_cast<uintptr>(raw)) {}

    /**
     * @brief constructor using default extended address
     */
    explicit address(const void * raw) : ext_(), raw_(reinterpret_cast<uintptr>(raw)) {}
#endif
    /**
     * @brief base constructor
     */
    explicit address(address_ext ext, uintptr raw) : ext_(ext), raw_(raw) {}

    /**
     * @brief constructor using default extended address
     */
    explicit address(uintptr raw) : ext_(), raw_(raw) {}

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

#ifndef HOST
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
#else
    /**
     * @brief updates the value pointed to by the reference
     */
    template <typename T, typename = std::enable_if<std::is_trivially_copyable<T>::value>>
    void write(const T& other) {
        // problem, what if stack is in dram???
        // maybe we make this only valid for scalar types...
        register T wv = other;
        register T* ptr = reinterpret_cast<T*>(raw_);
        {
            pod_address_guard grd(ext_.pod_addr());
            hb_mc_device_memcpy_to_device(the_device, raw_, &wv, sizeof(T));
        }
    }

    /**
     * @brief reads the value pointed to by the reference
     */
    template <typename T, typename = std::enable_if<std::is_trivially_copyable<T>::value>>
    T read() const {
        register T rv;
        register T* ptr = reinterpret_cast<T*>(raw_);
        {
            pod_address_guard grd(ext_.pod_addr());
            hb_mc_device_memcpy_to_host(the_device, &rv, raw_, sizeof(T));
        }
        return rv;
    }
#endif

    /**
     * @brief add operator
     */
    address operator+(uintptr off) const {
        return address(ext_, raw_+off);
    }

    /**
     * @brief subtract operator
     */
    address operator-(uintptr off) const {
        return address(ext_, raw_-off);
    }

    /**
     * @brief equality operator
     */
    bool operator==(const address& other) const {
        return ext_ == other.ext_ && raw_ == other.raw_;
    }

    FIELD(address_ext ,ext); //!< extended address information
    FIELD(uintptr     ,raw); //!< the raw pointer
};

}
#endif
