#include <cello/cello.hpp>
#include <util/statics.hpp>
#include "common.hpp"
#include "aes_kernel.hpp"

DRAM(ctx_vector_t)     ctx_v;
DRAM(message_vector_t) message_v;

int cello_main(int argc, char *argv[])
{
    message_v.foreach([](int i, message_t &dram_data){
        message_t dmem_data;
        // Copy from DRAM;
        message_t message = dram_data;
        AES_ctx ctx = ctx_v.local(i);
        
        // calculate call and put;
        AES_CBC_encrypt_buffer(&ctx, message.data(), message.size());

        // write back;
        dram_data = message;
    });
    return 0;
}
