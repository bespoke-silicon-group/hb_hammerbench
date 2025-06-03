#include <cello/cello.hpp>

int cello_main(int argc, char *argv[])
{
    cello_trace_on();
    //asm volatile ("add x0, x0, x0");
    cello::parallel_foreach(0, 1024, [](int i){
        asm volatile ("add x0, x0, x0");
    });
    cello_trace_off();
    return 0;
}
