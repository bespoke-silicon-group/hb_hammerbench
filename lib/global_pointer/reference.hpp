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
     * @brief constructor from address with default extended address
     */
    reference(void* raw): addr_(raw) {}

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

    /**
     * @brief set the pod x of the reference
     */
    reference& set_pod_x(unsigned x) {
        addr().set_pod_x(x);
        return *this;
    }

    /**
     * @brief set the pod y of the reference
     */
    reference& set_pod_y(unsigned y) {
        addr().set_pod_y(y);
        return *this;
    }


    FIELD(address, addr); //!< the address information
};

/**
 * generates the default constructors for the reference class
 */
#define BSG_GLOBAL_POINTER_REFERENCE_CONSTRUCTORS(type)                 \
    public:                                                             \
    /**                                                                 \
     * @brief base constructor                                          \
     */                                                                 \
    reference(address addr): addr_(addr) {}                             \
    /**                                                                 \
     * @brief constructor from address with default extended address    \
     */                                                                 \
    reference(void* raw): addr_(raw) {}                                 \
    /**                                                                 \
     * @brief default constructor                                       \
     */                                                                 \
    reference() : addr_() {}                                            \
    /**                                                                 \
     * @brief copy constructor for argument passing                     \
     */                                                                 \
    reference(const reference& other) : addr_(other.addr_) {}           \
    /**                                                                 \
     * @brief move constructor for return values                        \
     */                                                                 \
    reference(reference&& other) : addr_(std::move(other.addr_)) {}     \
    /**                                                                 \
     * @brief destructor                                                \
     */                                                                 \
    ~reference() {}                                                     \

/**
 * generates assignment operators for the reference class
 */
#define BSG_GLOBAL_POINTER_REFERENCE_ASSIGNMENT_OPERATORS(type)         \
    public:                                                             \
    /**                                                                 \
     * @brief copy assignment operator                                  \
     * this updates the value POINTED TO... NOT the reference object    \
     */                                                                 \
    reference& operator=(const reference& other) {                      \
        *this = (type)other;                                            \
        return *this;                                                   \
    }                                                                   \
    /**                                                                 \
     * @brief move assignment operator                                  \
     * this updates the value POINTED TO... NOT the reference object    \
     */                                                                 \
    reference& operator=(reference&& other) {                           \
        *this = (type)other;                                            \
        return *this;                                                   \
    }                                                                   \
    /**                                                                 \
     * @brief assignment operator                                       \
     * used as syntactic sugar for writes                               \
     * this updates the value POINTED TO... NOT the reference object    \
     */                                                                 \
    reference& operator=(const type& other) {                           \
        /* problem, what if stack is in dram???                         \
         * maybe we make this only valid for scalar types...            \
         */                                                             \
        write(other);                                                   \
        return *this;                                                   \
    }

/**
 * generates cast operators for the reference class
 */
#define BSG_GLOBAL_POINTER_REFERENCE_CAST_OPERATORS(type)               \
    public:                                                             \
    /**                                                                 \
     * @brief cast operator                                             \
     * used as syntactic sugar for reads                                \
     * this reads the value POINTED TO... NOT the reference object      \
     */                                                                 \
    operator type() const {                                             \
        return read();                                                  \
    }

/**
 * generates the write and read methods for the reference class
 */
#define BSG_GLOBAL_POINTER_REFERENCE_READ_WRITE_TRIVIAL(type)           \
    public:                                                             \
    /**                                                                 \
     * @brief updates the value pointed to by the reference             \
     */                                                                 \
    void write(const type& other) {                                     \
        /* problem, what if stack is in dram???                         \
         * maybe we make this only valid for scalar types...            \
         */                                                             \
        addr_.write(other);                                             \
    }                                                                   \
    /**                                                                 \
     * @brief reads the value pointed to by the reference               \
     */                                                                 \
    type read() const {                                                 \
        return addr_.read<type>();                                      \
    }                                                                   \

#define BSG_GLOBAL_POINTER_REFERENCE_INTERNAL(type)                     \
    public:                                                             \
    /**                                                                 \
     * @brief set the pod x of the reference                            \
     */                                                                 \
    reference& set_pod_x(unsigned x) {                                  \
        addr().set_pod_x(x);                                            \
        return *this;                                                   \
    }                                                                   \
    /**                                                                 \
     * @brief set the pod y of the reference                            \
     */                                                                 \
    reference& set_pod_y(unsigned y) {                                  \
        addr().set_pod_y(y);                                            \
        return *this;                                                   \
    }                                                                   \
    FIELD(address, addr); //!< the address information

/**
 * generates boiler plate for specialization of reference class
 */
#define BSG_GLOBAL_POINTER_REFERENCE_TRIVIAL(type)              \
    BSG_GLOBAL_POINTER_REFERENCE_CONSTRUCTORS(type)             \
    BSG_GLOBAL_POINTER_REFERENCE_ASSIGNMENT_OPERATORS(type)     \
    BSG_GLOBAL_POINTER_REFERENCE_CAST_OPERATORS(type)           \
    BSG_GLOBAL_POINTER_REFERENCE_READ_WRITE_TRIVIAL(type)       \
    BSG_GLOBAL_POINTER_REFERENCE_INTERNAL(type)

/**
 * generates a delegate method. this can be used to coalesce multiple accesses
 * without writing the samve value to pod address csr over and over again
 */
#define BSG_GLOBAL_POINTER_REFERENCE_METHOD(type, method, return_type)  \
    public:                                                             \
    return_type method() {                                              \
        type *p = reinterpret_cast<type*>(addr().raw());                \
        return_type r;                                                  \
        {                                                               \
            pod_address_guard grd(addr().ext().pod_addr());             \
            r = p->method();                                            \
        }                                                               \
        return r;                                                       \
    }
#define BSG_GLOBAL_POINTER_REFERENCE_METHOD_CONST(type, method, return_type) \
    public:                                                             \
    return_type method() const {                                        \
        type *p = reinterpret_cast<type*>(addr().raw());                \
        return_type r;                                                  \
        {                                                               \
            pod_address_guard grd(addr().ext().pod_addr());             \
            r = p->method();                                            \
        }                                                               \
        return r;                                                       \
    }

/**
 * generates accessors for a data member
 */
#define BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR_(type, accessor, member_type, member) \
    public:                                                             \
    reference<member_type> accessor() {                                 \
        return reference<member_type> (addr() + offsetof(type, member)); \
    }                                                                   \
    const reference<member_type> accessor() const {                     \
        return reference<member_type> (addr() + offsetof(type, member)); \
    }

/**
 * generates accessors for a data member, infers the type from the data member
 */
#define BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR(type, accessor, member)  \
    BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR_(type, accessor, decltype(std::declval<type>().member), member)

/**
 * generates accessor for a field declared using FIELD
 */
#define BSG_GLOBAL_POINTER_REFERENCE_FIELD(type, name)  \
    BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR(type, name, name##_)

}
#endif
