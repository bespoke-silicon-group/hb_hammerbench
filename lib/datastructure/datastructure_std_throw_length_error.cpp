#include <vector>
#include <bsg_manycore.h>

namespace std {
void __throw_length_error(const char* msg)
{
    bsg_print_hexadecimal(0xdeadbeef);
    bsg_fence();
    while (1);
}
}

