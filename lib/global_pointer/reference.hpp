#ifndef GLOBAL_POINTER_REFERENCE_HPP
#define GLOBAL_POINTER_REFERENCE_HPP
#include <stdint.h>
#include <utility>
#include <global_pointer/address.hpp>
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
    reference(address addr): addr_(addr) {}

    /**
     * @brief default constructor
     */
    reference() : addr_() {}

    /**
     * @brief copy constructor for argument passing
     */
    reference(const reference& other) : addr_(other.addr_) {}

    /**
     * @brief move constructor for return values
     */
    reference(reference&& other) : addr_(std::move(other.addr_)) {}

    /**
     * @brief destructor
     */
    ~reference() {}

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
        return *this;
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
        addr_.write(other);
    }

    /**
     * @brief reads the value pointed to by the reference
     */
    T read() const {
        return addr_.read<T>();
    }

    FIELD(address, addr); //!< the address information
};
}
#endif
