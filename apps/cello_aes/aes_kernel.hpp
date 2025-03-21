#include <bsg_manycore.h>
#include <stdint.h>
#include <stddef.h>
#include "common.hpp"

void AES_CBC_encrypt_buffer(struct AES_ctx *ctx, uint8_t * buf, size_t length);
