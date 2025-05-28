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
#ifdef NO_GRAIN_SCALING
    Vl.foreach(1, [](int i, int &v){
#else
    Vl.foreach([](int i, int &v){
#endif
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
