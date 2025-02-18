#ifndef UTIL_CONTAINER_OF_HPP
#define UTIL_CONTAINER_OF_HPP
#include <stddef.h>
#define container_of(ptr, type, member)                 \
    ((type *)((char *)(ptr) - offsetof(type, member)))

#endif
