#include <cello/cello.hpp>

extern "C" int setup()
{
    return 0;
}

int cello_main(int argc, char **argv)
{
    bsg_print_int(cello::my::pod_x());
    bsg_print_int(cello::my::pod_y());
    return 0;
}
