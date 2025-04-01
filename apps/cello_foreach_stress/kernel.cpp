#include <cello/cello.hpp>
#include <util/statics.hpp>
DRAM(int) niter = NITERS;
DRAM(std::atomic<int>) counter;
DRAM(int) data[NITERS];

int cello_main(int argc, char *argv[])
{
    int n = niter;
    cello::foreach(0, n, [](int i){
        data[i] = i;
        counter++;
    });
    return 0;
}
