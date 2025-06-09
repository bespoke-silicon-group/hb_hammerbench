#include <cello/cello.hpp>
#include <util/statics.hpp>
#include "common.hpp"
#include "aes_kernel.hpp"

#ifndef GRAIN_SCALE
#define GRAIN_SCALE 8
#endif

DRAM(ctx_vector_t)     ctx_v;
DRAM(message_vector_t) message_v;

int cello_main(int argc, char *argv[])
{
    int grain = message_v.local_size()/(cello::threads()*GRAIN_SCALE);
    if (grain < 1)
        grain = 1;

    bsg_print_int(message_v.size());
    bsg_print_int(grain);
    message_v.foreach(grain, [](int i, message_t &message){
#ifdef TRACE
        bsg_print_int(i);
#endif
        AES_ctx &ctx = ctx_v.local(i);
        AES_CBC_encrypt_buffer(&ctx, message.data(), message.size());
    });
    return 0;
}
