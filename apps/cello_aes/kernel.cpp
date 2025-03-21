#include <cello/cello.hpp>
#include <util/statics.hpp>
#include "common.hpp"
#include "aes_kernel.hpp"

DRAM(ctx_vector_t)     ctx_v;
DRAM(message_vector_t) message_v;

int cello_main(int argc, char *argv[])
{
    message_v.foreach([](int i, message_t &message){
#ifdef TRACE
        bsg_print_int(i);
#endif
        AES_ctx &ctx = ctx_v.local(i);
        AES_CBC_encrypt_buffer(&ctx, message.data(), message.size());
    });
    return 0;
}
