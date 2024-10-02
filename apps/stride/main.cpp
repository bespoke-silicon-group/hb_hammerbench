#include <cstdlib>
#include <cstring>
#include "bsg_manycore_cuda.h"
#include "bsg_manycore_regression.h"
#include "HammerBlade.hpp"

using namespace hammerblade;
using namespace host;

int Stride(int argc, char *argv[])
{
    auto hb = HammerBlade::Get();
    const char *app = argv[1];
    int s = atoi(argv[2]);
    int n = atoi(argv[3]);
    int x = atoi(argv[4]);
    int y = atoi(argv[5]);
    std::cout
        << "stride-size = " << s << " "
        << "stride-n = " << n << " "
        << "x = " << x << " "
        << "y = " << y << " "
        << std::endl;
    hb->load_application(argv[1]);
    hb_mc_eva_t A_ptr = hb->alloc(sizeof(int)*(n*s+1));
#ifdef WARMUP
    std::cout << "warming up" << std::endl;
    hb->push_job(Dim{1,1}, hb->physical_dimension(), "warmup", A_ptr, s, n);
    hb->exec();
#endif
    std::cout << "starting stride" << std::endl;
    hb->push_job(Dim{1,1}, hb->physical_dimension(), "stride", A_ptr, s, n, x, y);
    hb->exec();
    hb->close();
    return HB_MC_SUCCESS;
}

declare_program_main("Stride", Stride);
