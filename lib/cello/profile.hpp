#ifndef CELLO_PROFILE_HPP
#define CELLO_PROFILE_HPP
#ifndef HOST
#include <bsg_manycore.h>
#endif

#define CELLO_MMIO_EPA 0xEB00
#define CELLO_TRACE_TOGGLE_EPA  (CELLO_MMIO_EPA+0)

#ifndef HOST
#define cello_trace_on()                        \
    do { *(int*)bsg_remote_ptr_io(IO_X_INDEX, CELLO_TRACE_TOGGLE_EPA) = 1; bsg_fence(); } while (0)
#define cello_trace_off()                       \
    do { *(int*)bsg_remote_ptr_io(IO_X_INDEX, CELLO_TRACE_TOGGLE_EPA) = 0; bsg_fence(); } while (0)
#endif
#endif
