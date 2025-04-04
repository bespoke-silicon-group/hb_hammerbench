#include <cello/cello.hpp>
#include <util/statics.hpp>
#include <algorithm>
#include "common.hpp"

DRAM(csx_type) A;
DRAM(csx_type) B;
DRAM(csx_type) C;
//DRAM(list_head_vector) C_list_head;
DRAM(count_vector) C_col_count;

#define fmadd_asm(rd_p, rs1_p, rs2_p, rs3_p)                            \
    asm volatile ("fmadd.s %[rd], %[rs1], %[rs2], %[rs3]"               \
                  : [rd] "=f" (rd_p)                                    \
                  : [rs1] "f" ((rs1_p)), [rs2] "f" ((rs2_p)), [rs3] "f" ((rs3_p)))


int cello_main(int argc, char *argv[])
{
    bsg_printf("A: %d rows, %d columns, %d non-zeros\n", A.rows(), A.cols(), A.nnz());
    bsg_printf("B: %d rows, %d columns, %d non-zeros\n", B.rows(), B.cols(), B.nnz());
    C_col_count.foreach([](int i, index_type &count) {
        auto [A_idx_start, A_idx_end, A_val_start, A_val_end] = A.inner_indices_values_range_lcl(i);        
        asm volatile ("" ::: "memory");
        
        value_type *A_val_p = A_val_start;
        for (index_type *A_idx_p = A_idx_start;
             A_idx_p != A_idx_end;
             A_idx_p++, A_val_p++) {
            index_type A_idx = *A_idx_p;
            value_type A_val = *A_val_p;
            asm volatile ("" ::: "memory");

            auto [B_idx_start, B_idx_end, B_val_start, B_val_end] = B.inner_indices_values_range(A_idx);
            asm volatile ("" ::: "memory");

            auto B_val_p = B_val_start;
            for (auto B_idx_p = B_idx_start;
                 B_idx_p != B_idx_end;
                 B_idx_p++, B_val_p++) {
                index_type B_idx = *B_idx_p;
                value_type B_val = *B_val_p;
                asm volatile ("" ::: "memory");

                bsg_print_int(1000000000 + 1000000*i + 1000*A_idx + B_idx);
                bsg_fence();
            }
        }

    });
    return 0;
}
