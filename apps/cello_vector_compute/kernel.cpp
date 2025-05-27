#include <cello/cello.hpp>
#include <util/statics.hpp>
#include "common.hpp"

DRAM(vec) V;
DMEM(vec) Vl;

static void setup()
{
    Vl = V;
    return;
}

cello_constructor(setup);

int cello_main(int argc, char *argv[])
{
    Vl.foreach(1, [](int i, int &v){
        for (int j = 0; j < COMPUTE; j++) {
            asm volatile("addi x0, x0, 1");
            asm volatile("addi x0, x0, 1");
            asm volatile("addi x0, x0, 1");
            asm volatile("addi x0, x0, 1");
            asm volatile("addi x0, x0, 1");
            asm volatile("addi x0, x0, 1");
            asm volatile("addi x0, x0, 1");
            asm volatile("addi x0, x0, 1");
        }
    });
    return 0;
}
