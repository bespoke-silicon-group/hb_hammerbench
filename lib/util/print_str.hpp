#ifndef UTIL_PRINT_STR_HPP
#define UTIL_PRINT_STR_HPP
#include "bsg_manycore.h"
namespace util
{
static inline void print_str(const char *str)
{
        while (*str) { bsg_putchar(*str++); }
}
}
#endif
