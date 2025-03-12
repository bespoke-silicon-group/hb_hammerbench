#ifndef DATA_H
#define DATA_H
#include <util/class_field.hpp>
#include <global_pointer/global_pointer.hpp>

struct data_t {
    FIELD(int, a);
    FIELD(int, b);
    FIELD(int, c);
    FIELD(int, d);
    int sum() { return a() + b() + c() + d(); }
};

template <>
class bsg_global_pointer::reference<data_t> {
    BSG_GLOBAL_POINTER_REFERENCE_TRIVIAL(data_t);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(data_t, a);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(data_t, b);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(data_t, c);
    BSG_GLOBAL_POINTER_REFERENCE_FIELD(data_t, d);    
};

#endif
