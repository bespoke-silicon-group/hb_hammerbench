#include <cello/cello.hpp>
#include <util/statics.hpp>
#include "common.hpp"
#include "aes_kernel.hpp"

#ifndef GRAIN_SCALE
#define GRAIN_SCALE 8
#endif

#define TRACE
#ifdef TRACE
#define trace(x) \
    bsg_print_int(x)
#else
#define trace(x)
#endif

DRAM(ctx_vector_t)     ctx_v;
DRAM(message_vector_t) message_v;

template <typename T>
using gref = bsg_global_pointer::reference<T>;

using guard = bsg_global_pointer::pod_address_guard;

int cello_main(int argc, char *argv[])
{
    int grain = message_v.local_size()/(cello::threads()*GRAIN_SCALE);
    if (grain < 1)
        grain = 1;

    bsg_print_int(message_v.size());
    bsg_print_int(grain);
#ifdef CELLO_GLOBAL_STEALING
    message_v.foreach_unrestricted(grain, [](int i, gref<message_t> message) {
        trace(i);
        gref<AES_ctx> ctx = ctx_v.at(i);
        {
            guard _ (ctx.addr().ext().pod_addr());
            AES_ctx *ctx_lcl = (AES_ctx*)ctx.addr().raw();
            message_t* msg_lcl = (message_t*)message.addr().raw();
            AES_CBC_encrypt_buffer(ctx_lcl, msg_lcl->data(), msg_lcl->size());
        }
    });
#else
    message_v.foreach(grain, [](int i, message_t &message){
        trace(i);
        AES_ctx &ctx = ctx_v.local(i);
        AES_CBC_encrypt_buffer(&ctx, message.data(), message.size());
    });
#endif
    return 0;
}
