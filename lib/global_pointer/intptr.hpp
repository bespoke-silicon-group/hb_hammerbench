#ifndef GLOBAL_POINTER_INT_PTR_H
#define GLOBAL_POINTER_INT_PTR_H
#ifndef HOST
#include <cstdint>
#else
#include <bsg_manycore_cuda.h>
#endif
namespace bsg_global_pointer {
#ifndef HOST
typedef uintptr_t uintptr;
#else
typedef hb_mc_eva_t uintptr;
#endif
}
#endif
