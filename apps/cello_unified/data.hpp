#ifndef DATA_H
#define DATA_H
#include <util/class_field.hpp>
#include <global_pointer/global_pointer.hpp>

struct inner_t {
    FIELD(int, x);
    FIELD(int, y);
};
    
struct data_t {
    FIELD(int, a);
    FIELD(int, b);
    FIELD(int, c);
    FIELD(int, d);
    FIELD(inner_t, i);
    int sum() const { return a() + b() + c() + d(); }
};

template <>
class bsg_global_pointer::reference<inner_t> {
    BSG_GLOBAL_POINTER_REFERENCE_TRIVIAL(inner_t);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(inner_t, x);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(inner_t, y);
};

template <>
class bsg_global_pointer::reference<data_t> {
    BSG_GLOBAL_POINTER_REFERENCE_TRIVIAL(data_t);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(data_t, a);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(data_t, b);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(data_t, c);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(data_t, d);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(data_t, i);
    BSG_GLOBAL_POINTER_REFERENCE_FUNCTION_CONST(data_t, sum, int);
};

#endif
