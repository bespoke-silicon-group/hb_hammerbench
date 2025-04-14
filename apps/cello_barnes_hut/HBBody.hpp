#pragma once
#include <array>
#include <global_pointer/global_pointer.hpp>

typedef struct HBBody_ {
    float mass;
    std::array<float, 3> pos;
    std::array<float, 3> vel;
    std::array<float, 3> acc;
} HBBody;

template <>
class bsg_global_pointer::reference<HBBody> {
    BSG_GLOBAL_POINTER_REFERENCE_TRIVIAL(HBBody);
    BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR(HBBody, Mass, mass);
    BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR(HBBody, Pos_, pos);
    BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR(HBBody, Vel_, vel);
    BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR(HBBody, Acc_, acc);
    template <typename Idx, typename = typename std::enable_if<std::is_integral<Idx>::value>::type>    
    bsg_global_pointer::reference<float> Pos(Idx i) {
        auto array_ptr = bsg_global_pointer::addressof(Pos_());
        auto fptr = bsg_global_pointer::pointer_cast<float>(array_ptr);
        return fptr[i];
    }
    template <typename Idx, typename = typename std::enable_if<std::is_integral<Idx>::value>::type>    
    bsg_global_pointer::reference<float> Vel(Idx i) {
        auto array_ptr = bsg_global_pointer::addressof(Vel_());
        auto fptr = bsg_global_pointer::pointer_cast<float>(array_ptr);
        return fptr[i];
    }
    template <typename Idx, typename = typename std::enable_if<std::is_integral<Idx>::value>::type>    
    bsg_global_pointer::reference<float> Acc(Idx i) {
        auto array_ptr = bsg_global_pointer::addressof(Acc_());
        auto fptr = bsg_global_pointer::pointer_cast<float>(array_ptr);
        return fptr[i];
    }    
};
