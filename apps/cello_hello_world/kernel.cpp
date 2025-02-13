#include <cello/cello.hpp>
#include <bsg_manycore.h>

extern "C" int setup(unsigned pod_x, unsigned pod_y)
{
    return 0;
}

int cello_main(int argc, char *argv[])
{
    bsg_printf("hello world\n");
    return 0;
}
