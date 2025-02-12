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

    /**
     * @brief constructor from address with default extended address
     */
    pointer(void* raw) : ref_(raw) {}

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
    pointer(pointer&& other) : ref_(std::move(other.ref_)) {}

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
        ref_.addr() = std::move(other.ref_.addr());
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

    /**
     * @brief set the pod y of the pointer
     */
    pointer<T>& set_pod_y(unsigned y) {
        ref().set_pod_y(y);
        return *this;
    }

    FIELD(reference<T>, ref); //!< reference object
};

/**
 * @brief specialize reference to a pointer
 * TODO
 */

}
#endif
