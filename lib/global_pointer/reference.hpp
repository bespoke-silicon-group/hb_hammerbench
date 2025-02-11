#ifndef GLOBAL_POINTER_REFERENCE_HPP
#define GLOBAL_POINTER_REFERENCE_HPP
#include <global_pointer/address_ext.hpp>
#include <stdint.h>
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
    address_ext   ext_; //!< extended address information
    T*            raw_; //!< the raw pointer
};

}
#endif
