#include <stdint.h>
#include "bsg_manycore.h"
#include "appl.hpp"

int32_t fib_base(int32_t n) {
  if (n < 2)
    return n;
  else
    return fib_base(n-1) + fib_base(n-2);
}

int32_t fib(int32_t n, int32_t gsize = 2) {
  bsg_print_int(1000000+n);
  // unsigned sp;
  // asm volatile("mv %0, sp" : "=r"(sp));
  // bsg_print_hexadecimal(sp);
  if (n <= gsize) {
    int r = fib_base(n);
    bsg_print_int(2000000+n);
    return r;
  }

  int32_t x, y;

  appl::parallel_invoke(
      [&] { x = fib(n-1, gsize); },
      [&] { y = fib(n-2, gsize); }
      );

  bsg_print_int(2000000+n);
  return x + y;
}

extern "C" __attribute__ ((noinline))
int kernel_appl_fib(int* results, int n, int grain_size, int* dram_buffer) {

  // debug print
  if (__bsg_id == 0) {
    bsg_print_int(n);
    bsg_print_int(grain_size);
  }

  // output
  int32_t result     = -1;

#ifdef TRACE_KERNEL
  bsg_print_int(91);
#endif
  // --------------------- kernel ------------------------
  appl::runtime_init(dram_buffer, 2);

#ifdef TRACE_KERNEL
  bsg_print_int(92);
#endif
  // sync
  appl::sync();
  bsg_cuda_print_stat_kernel_start();

  if (__bsg_id == 0) {
    result = fib(n, grain_size);
    results[0] = result;
  } else {
    appl::worker_thread_init();
  }
#ifdef TRACE_KERNEL  
  bsg_print_int(93);
#endif
  appl::runtime_end();
  // --------------------- end of kernel -----------------

#ifdef TRACE_KERNEL  
  bsg_print_int(94);
#endif
  bsg_cuda_print_stat_kernel_end();
  bsg_print_int(result);

  appl::sync();
#ifdef TRACE_KERNEL
  bsg_print_int(95);
#endif

  return 0;
}
