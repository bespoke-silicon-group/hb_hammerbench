#include <cello/cello.hpp>
#include <util/statics.hpp>
#include "common.hpp"

DRAM(csx_type) A;
DRAM(csx_type) B;
DRAM(csx_type) C;
DRAM(list_head_vector) C_list_head;

int cello_main(int argc, char *argv[])
{
    bsg_printf("A: %d rows, %d columns, %d non-zeros\n", A.rows(), A.cols(), A.nnz());
    bsg_printf("B: %d rows, %d columns, %d non-zeros\n", B.rows(), B.cols(), B.nnz());
    C_list_head.foreach([](int i, HBListNodePtr &row_result) {
        unsigned *sp;
        asm volatile ("mv %0, sp" : "=r"(sp));
        bsg_print_hexadecimal((unsigned long)sp);
    });
    return 0;
}
