#include <cello/cello.hpp>
#include <util/statics.hpp>
#include "common.hpp"

DRAM(vec) V;

int cello_main(int argc, char *argv[])
{
    V.foreach([](int i, int &v){
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
