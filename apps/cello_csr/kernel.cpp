#include <cello/cello.hpp>
#include <util/statics.hpp>
#include "common.hpp"

DRAM(csx_type) matrix;
DRAM(vector_type) outer_sums;

#define info(fmt, ...)                          \
    bsg_printf("[%d,%d]: " fmt "\n", cello::my::pod_x(), cello::my::pod_y(), ##__VA_ARGS__)
        
int cello_main(int argc, char *argv[])
{
#define USE_LOCAL
#ifdef  USE_LOCAL
    outer_sums.foreach([](int row, float &rsum){
        //bsg_print_int(1000000+row);
        float sum = 0;
        auto [start, end] = matrix.values_range_lcl(row);
        for (auto i = start; i != end; i++) {
            sum += *i;
        }
        rsum = sum;
    });
#else
    cello::parallel_foreach(0, (int)matrix.rows(), [](int row) {
        float sum = 0;
        //bsg_print_int(1000000+row);
        auto [start, end] = matrix.values_range(row);
        for (auto i = start; i != end; i++) {
            sum += *i;
        }
        outer_sums[row] = sum;
    });
#endif
    return 0;
}
