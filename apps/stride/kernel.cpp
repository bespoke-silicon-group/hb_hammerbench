#include <bsg_manycore.h>
#include <bsg_cuda_lite_barrier.h>

#ifndef STRIDE_SIZE
#error "Define STRIDE_SIZE"
#endif
#ifndef ILP
#error "Define ILP"
#endif
#ifndef LOADS
#error "Define LOADS"
#endif
extern "C" int stride(int *A)
{
    bsg_cuda_print_stat_kernel_start();
    int i = 0;
    int s = 0;
    for(; i+ILP <= LOADS; i+=ILP) {
        register int a[ILP];
        bsg_unroll(8)
        for (int ilp = 0; ilp < ILP; ilp++) {
            a[ilp] = A[(i+ilp)*STRIDE_SIZE];            
        }
        bsg_compiler_memory_barrier();
        for (int ilp = 0; ilp < ILP; ilp++) {
            s += a[ilp];
        }
    }
    for (; i < LOADS; i++) {
        s += A[i];
    }
    bsg_cuda_print_stat_kernel_end();
    bsg_fence();
    A[0] = s;
    return 0;
}
