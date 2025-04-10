#ifndef HB_LIST_HPP
#define HB_LIST_HPP
#include <cstdint>
#ifdef HOST
#include <bsg_manycore_cuda.h>
#endif

struct HBListNode_;

#ifndef HOST
using HBListNodePtr = HBListNode_*;
#else
using HBListNodePtr = hb_mc_eva_t;
#endif

typedef struct HBListNode_ {
    int32_t col_idx;
    float nnz;
    HBListNodePtr next;
} HBListNode;


typedef struct HBList_ {
    int32_t count;
    HBListNodePtr head;
    HBListNodePtr tail;
} HBList;

#endif
