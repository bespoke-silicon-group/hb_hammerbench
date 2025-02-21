#ifndef UTIL_STATICS_HPP
#define UTIL_STATICS_HPP

#define DRAM(type)                                 \
    __attribute__((section(".dram"))) type

#define DMEM(type)                                 \
    __attribute__((section(".dmem"))) type

#endif
