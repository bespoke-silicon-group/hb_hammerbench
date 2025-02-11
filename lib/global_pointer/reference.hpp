#ifndef GLOBAL_POINTER_REFERENCE_HPP
#define GLOBAL_POINTER_REFERENCE_HPP
#include <global_pointer/address_ext.hpp>
#include <stdint.h>
#include <utility>
namespace bsg_global_pointer
{
/**
 * forward declaration of pointer
 */
template <typename T>
class pointer;

/**
 * @brief default reference class suitable for most builtin types
 */
template <typename T>
class reference
{    
public:
    /**
     * @brief base constructor
     */
    reference(address_ext ext, T* raw) : ext_(ext), raw_(raw) {}

    /**
     * @brief constructor using default extended address
     */
    reference(T* raw) : ext_(), raw_(raw) {}

    /**
     * @brief constructor using default extended address and null pointer
     */
    reference() : ext_(), raw_(nullptr) {}

    /**
     * @brief copy constructor for argument passing
     */
    reference(const reference& other) : ext_(other.ext_), raw_(other.raw_) {}

    /**
     * @brief move constructor for return values
     */
    reference(reference&& other) : ext_(other.ext_), raw_(other.raw_) {}

    /**
     * @brief copy assignment operator
     * this updates the value POINTED TO... NOT the reference object
     */
    reference& operator=(const reference& other) {
        *this = (T)other;
        return *this;
    }

    /**
     * @brief move assignment operator
     * this updates the value POINTED TO... NOT the reference object
     */
    reference& operator=(reference&& other) {
        *this = (T)other;
        return *this;
    }

    /**
     * @brief assignment operator
     * used as syntactic sugar for writes
     * this updates the value POINTED TO... NOT the reference object
     */
    reference& operator=(const T& other) {
        // problem, what if stack is in dram???
        // maybe we make this only valid for scalar types...
        write(other);
    }

    /**
     * @brief cast operator
     * used as syntactic sugar for reads
     * this reads the value POINTED TO... NOT the reference object
     */
    operator T() const {
        return read();
    }

    /**
     * @brief updates the value pointed to by the reference
     */
    void write(const T& other) {
        // problem, what if stack is in dram???
        // maybe we make this only valid for scalar types...
        register T wv = other;
        {
            pod_address_guard grd(ext_.pod_addr());
            *raw_ = wv;
        }
    }

    /**
     * @brief reads the value pointed to by the reference
     */
    T read() const {
        register T rv;
        {
            pod_address_guard grd(ext_.pod_addr());
            rv = *raw_;
        }
        return rv;
    }

    FIELD(address_ext ,ext); //!< extended address information
    FIELD(T*          ,raw); //!< the raw pointer

};
}
#endif
