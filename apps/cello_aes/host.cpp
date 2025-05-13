#include <cello/host/cello.hpp>
#include <vector>
#include <cstring>
#include "common.hpp"
#include "aes.hpp"

class program : public cello::program
{
    int init(int argc, char *argv[]) override {
        cello::program::init(argc, argv);

        ctx_v = new ctx_vector_t::mirror_type(find<ctx_vector_t>("ctx_v"));
        message_v = new message_vector_t::mirror_type(find<message_vector_t>("message_v"));
        
        ctx_data.resize(NUM_ITERS);
        memset(ctx_data.data(), 0, ctx_data.size() * sizeof(AES_ctx));
        for (size_t i = 0; i < NUM_ITERS; i++) {
            AES_init_ctx(&ctx_data[i], key.data());
        }
        ctx_v->init_host_from(ctx_data);
        
        for (size_t j = 0; j < message.size(); j++) {
            message[j] = j;
        }

        message_data.resize(NUM_ITERS);
        for (size_t i = 0; i < NUM_ITERS; i++) {
            message_data[i] = message;
        }
        message_v->init_host_from(message_data);

        
        expected_message = message;        
        memset(&expected_ctx, 0, sizeof(AES_ctx));

        AES_init_ctx(&expected_ctx, key.data());
        AES_CBC_encrypt_buffer(&expected_ctx, expected_message.data(), expected_message.size());
        return 0;
    }
    
    int input() override {
        cello::program::input();
        BSG_CUDA_CALL(ctx_v->sync_device(jobs_in));
        BSG_CUDA_CALL(message_v->sync_device(jobs_in));
        return 0;
    }

    int output() override {
        cello::program::output();
        BSG_CUDA_CALL(ctx_v->sync_host(jobs_out));
        BSG_CUDA_CALL(message_v->sync_host(jobs_out));
        return 0;
    }

    int check_output() override {
        cello::program::check_output();
        bool fail = false;
        for (size_t i = 0; i < message_data.size(); i++) {
            for (size_t j = 0; j < message.size(); j++) {
                uint8_t expected = expected_message[j];
                uint8_t actual = message_v->at(i)[j];
                if (expected != actual) {
                    printf("Mismatch: i=%zu, j=%zu, actual=%u, expected=%u\n", i, j, actual, expected);
                    fail = true;
                }
            }
        }
        delete ctx_v;
        delete message_v;
        return fail ? HB_MC_FAIL : HB_MC_SUCCESS;
    }

    int num_iters = NUM_ITERS;
    aes_key_t key = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};
    std::vector<AES_ctx> ctx_data;
    std::vector<message_t> message_data;
    AES_ctx expected_ctx;
    message_t message;
    message_t expected_message;
    ctx_vector_t::mirror_type *ctx_v;
    message_vector_t::mirror_type *message_v;
};

cello::program* make_program() {
    return new program();
}
