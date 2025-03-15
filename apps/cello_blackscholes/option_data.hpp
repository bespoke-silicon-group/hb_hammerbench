#pragma once
#include <global_pointer/global_pointer.hpp>
typedef struct OptionData_ {
        // Inputs
        float s;          // spot price
        float strike;     // strike price
        float r;          // risk-free interest rate
        float v;          // volatility
        float t;          // time to maturity or option expiration in years 
        // Outputs;
        float call;
        float put;
        float unused;
        int   completed;
} OptionData;

template <>
class bsg_global_pointer::reference<OptionData> {
public:
    BSG_GLOBAL_POINTER_REFERENCE_TRIVIAL(OptionData);
    BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR(OptionData, S, s);
    BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR(OptionData, STRIKE, strike);
    BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR(OptionData, R, r);
    BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR(OptionData, V, v);
    BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR(OptionData, T, t);
    BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR(OptionData, CALL, call);
    BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR(OptionData, PUT, put);
    BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR(OptionData, UNUSED, unused);
    BSG_GLOBAL_POINTER_REFERENCE_ACCESSOR(OptionData, COMPLETED, completed);
};
