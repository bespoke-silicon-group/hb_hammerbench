//========================================================================
// hw-barrier.h
//========================================================================

#ifndef APPL_HW_BARRIER_H
#define APPL_HW_BARRIER_H

#include "bsg_manycore.h"
// #ifdef ENABLE_HW_BARRIER
// #undef ENABLE_HW_BARRIER
// #endif
#include "bsg_cuda_lite_barrier.h"
#include "bsg_barrier_amoadd.h"

namespace appl {

__attribute__((weak, section((".dmem"))))
int sense = 1;

inline void config_hw_barrier() {
  //bsg_barrier_tile_group_init();
}

inline void sync() {
  bsg_fence();
  //bsg_barrier_tile_group_sync();
  // use the amoadd barrier explicitly
  bsg_barrier_amoadd(&__cuda_barrier_cfg[0], &sense);  
  bsg_fence();
}

} // namespace appl

#endif
