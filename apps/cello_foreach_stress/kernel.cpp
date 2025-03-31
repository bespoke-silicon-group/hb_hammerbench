#include <cello/cello.hpp>
#include <util/statics.hpp>
DRAM(int) niter = NITERS;
int cello_main(int argc, char *argv[])
{
    int n = niter;
    cello::foreach(0, n, [](int i){
        bsg_print_int(i);
    });
    return 0;
}
