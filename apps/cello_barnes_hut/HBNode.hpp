#pragma once
#include <stdint.h>
#include <array>
#include <global_pointer/global_pointer.hpp>

typedef struct HBNode_ {
    // original kernel makes these pointers;
    // we will make them integers
    // lsb indicates if this is an index into bodies
    // lsb = 1 -> index into bodies vector
    // lsb = 0 -> index into nodes vector
#define leaf 0x80000000
    std::array<uint32_t, 8> child;
    float co_mass;
    std::array<float, 3> co_pos;
    float diamsq;
} HBNode;


template <>
class bsg_global_pointer::reference<HBNode> {
    BSG_GLOBAL_POINTER_REFERENCE_TRIVIAL(HBNode);
    BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR(HBNode, Child_, child);
    BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR(HBNode, CoPos_, co_pos);
public:
    template <typename Idx, typename = typename std::enable_if<std::is_integral<Idx>::value>::type>    
    bsg_global_pointer::reference<uint32_t> Child(Idx i) {
        auto array_ptr = bsg_global_pointer::addressof(Child_());
        auto uptr = bsg_global_pointer::pointer_cast<uint32_t>(array_ptr);
        return uptr[i];
    }
    template <typename Idx, typename = typename std::enable_if<std::is_integral<Idx>::value>::type>    
    bsg_global_pointer::reference<float> CoPos(Idx i) {
        auto array_ptr = bsg_global_pointer::addressof(CoPos_());
        auto fptr = bsg_global_pointer::pointer_cast<float>(array_ptr);
        return fptr[i];
    }
    BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR(HBNode, CoMass, co_mass);
    BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR(HBNode, DiamSq, diamsq);
};
