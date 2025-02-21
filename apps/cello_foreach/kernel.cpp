#include <cello/cello.hpp>

extern "C" int setup()
{
    return 0;
}

extern "C" int cello_main(int argc, char **argv)
{
    using namespace cello;
    cello::parallel_foreach(0, 64, [](int i) {
        bsg_print_int(i);
    });
    return 0;
}
