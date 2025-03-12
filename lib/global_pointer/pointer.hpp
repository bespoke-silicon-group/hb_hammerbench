#ifndef GLOBAL_POINTER_POINTER_HPP
#define GLOBAL_POINTER_POINTER_HPP
#include <global_pointer/reference.hpp>
#include <util/class_field.hpp>
#include <global_pointer/address.hpp>
namespace bsg_global_pointer
{
/**
 * @brief pointer class
 * similar to a reference, except provides the syntax of a pointer
 */
template <typename T>
class pointer
{
public:
    /**
     * @brief base constructor
     */
    pointer(address addr) : ref_(addr) {}

#ifndef HOST
    /**
     * @brief constructor from address with default extended address
     */
    pointer(const void* raw) : ref_(raw) {}
#endif

    /**
     * @brief constructor from uintptr
     */
    pointer(uintptr raw) : ref_(raw) {}

    /**
     * @brief default constructor
     */
    pointer() : ref_() {}

    /**
     * @brief copy constructor for argument passing
     */
    pointer(const pointer& other) : ref_(other.ref_) {}

    /**
     * @brief move constructor for return values
     */
    pointer(pointer&& other) : ref_(other.ref_) {}

    /**
     * @brief make a global pointer from a local pointer and pod coordinates
     */
    static pointer<T> onPodXY(unsigned x, unsigned y, const T* ptr) {
        return pointer<T>
            (address(address_ext(pod_address(x,y)), ptr));
    }

    /**
     * @brief make a global pointer from a local pointer and an extended address
     */
    static pointer<T> withExtAddr(const address_ext& ext, const T* ptr) {
        return pointer<T>(address(ext, ptr));
    }

    /**
     * @brief copy assignment operator
     */
    pointer& operator=(const pointer& other) {
        ref_.addr() = other.ref_.addr();
        return *this;
    }

    /**
     * @brief move assignment operator
     */
    pointer& operator=(pointer&& other) {
        ref_.addr() = other.ref_.addr();
        return *this;
    }

    /**
     * @brief destructor
     */
    ~pointer() {}

    /**
     * @brief writable dereference operator
     */
    reference<T> operator*() {
        return ref_;
    }

    /**
     * @brief read-only dereference operator
     */
    const reference<T> operator*() const {
        return ref_;
    }

    /**
     * @brief arrow operator
     */
    reference<T>* operator->() {
        return &ref_;
    }

    /**
     * @brief const arrow operator
     */
    const reference<T>* operator->() const {
        return &ref_;
    }

    /**
     * @brief indexing operator
     */
    template <typename I>
    reference<T> operator[](I i) {
        return reference<T>(ref_.addr() + (i * sizeof(T)));
    }

    /**
     * @brief const indexing operator
     */
    template <typename I>
    const reference<T> operator[](I i) const {
        return reference<T>(ref_.addr() + (i * sizeof(T)));
    }

    /**
     * @brief set the pod x of the pointer
     */
    pointer<T>& set_pod_x(unsigned x) {
        ref().set_pod_x(x);
        return *this;
    }

    unsigned pod_x() const {
        return ref().pod_x();
    }

    /**
     * @brief set the pod y of the pointer
     */
    pointer<T>& set_pod_y(unsigned y) {
        ref().set_pod_y(y);
        return *this;
    }

    unsigned pod_y() const {
        return ref().pod_y();
    }

    /**
     * @brief equality operator
     */
    bool operator==(const pointer<T>& other) const {
        return ref_.addr() == other.ref_.addr();
    }

    /**
     * @brief inequality operator
     */
    bool operator!=(const pointer<T>& other) const {
        return !(*this == other);
    }

    FIELD(reference<T>, ref); //!< reference object
};

/**
 * @brief specialize reference to a pointer
 */
template <typename T>
class reference<pointer<T>>
{
    BSG_GLOBAL_POINTER_REFERENCE_TRIVIAL(pointer<T>);
    /**
     * @brief dereference operator
     */
    reference<T> operator*() {
        pointer<T> ptr = this->read();
        return *ptr;
    }

    /**
     * @brief const dereference operator
     */
    const reference<T> operator*() const {
        pointer<T> ptr = read();
        return *ptr;
    }

    /**
     * @brief arrow operator
     */
    pointer<T> operator->() {
        return this->read();
    }

    /**
     * @brief const arrow operator
     */
    const pointer<T> operator->() const {
        return this->read();
    }
};

template <typename To, typename From>
pointer<To> pointer_cast(const pointer<From>& from) {
    return pointer<To>(from.ref().addr());
}
}
#endif
