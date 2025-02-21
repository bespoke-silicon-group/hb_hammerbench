#include <cello/cello.hpp>
#include <atomic>
#include <util/test_eq.hpp>
#include <util/statics.hpp>

extern "C" int setup()
{
    return 0;
}


DRAM(std::atomic<int>) sum, mask;

extern "C" int cello_main(int argc, char **argv)
{
    using namespace cello;
    mask = (1 << __bsg_id);
    sum = 0;
    cello::parallel_foreach(0, 64, [](int i) {
        sum += i;
        mask |= (1 << __bsg_id);
    });
    TEST_EQ(INT, sum, 2016);
    TEST_NEQ(INT, mask, (1 << __bsg_id));

    mask = (1 << __bsg_id);
    sum = 0;
    cello::foreach<cello::serial>(0, 64, [](int i) {
        sum += i;
        mask |= (1 << __bsg_id);
    });
    TEST_EQ(INT, sum, 2016);
    TEST_EQ(INT, mask, (1 << __bsg_id));
    return 0;
}
