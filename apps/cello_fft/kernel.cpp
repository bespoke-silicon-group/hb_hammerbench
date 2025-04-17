#include <cello/cello.hpp>

int cello_main(int argc, char *argv[])
{
    bsg_print_int(cello::my::pod_id());
    bsg_print_int(cello::my::pod_x());
    bsg_print_int(cello::my::pod_y());
    return 0;
}
