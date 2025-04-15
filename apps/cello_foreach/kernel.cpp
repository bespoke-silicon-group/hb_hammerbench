#include <cello/cello.hpp>
#include <atomic>
#include <util/test_eq.hpp>
#include <util/statics.hpp>
#include <global_pointer/global_pointer.hpp>
DRAM(std::atomic<int>) sum, mask;
static constexpr int N = 128;
extern "C" int cello_main(int argc, char **argv)
{
    using namespace cello;
    mask = (1 << __bsg_id);
    sum = 0;
    cello::parallel_foreach(0, N, [](int i) {
        unsigned sp;
        asm volatile ("mv %0, sp" : "=r"(sp));
        bsg_print_hexadecimal(sp);
        
        bsg_global_pointer::pod_address origin;
        origin.set_pod_x(0).set_pod_y(0);
        bsg_global_pointer::pod_address_guard grd(origin);
        sum += i;
        mask |= (1 << __bsg_id);
    });
    TEST_EQ(INT, sum, N*(N-1)/2);
    TEST_NEQ(INT, mask, (1 << __bsg_id));

    mask = (1 << __bsg_id);
    sum = 0;
    cello::foreach<cello::serial>(0, N, [](int i) {
        sum += i;
        mask |= (1 << __bsg_id);
    });
    TEST_EQ(INT, sum, N*(N-1)/2);
    TEST_EQ(INT, mask, (1 << __bsg_id));
    return 0;
}
