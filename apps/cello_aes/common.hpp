#pragma once
#include <array>
#include <datastructure/vector.hpp>
#ifdef HOST
extern "C" {
#include "aes.h"
}
#endif

// AES128
#define AES_BLOCKLEN 16
#define Nb 4
#define Nk 4
#define Nr 10
#define xtime(x) ((x<<1) ^ (((x>>7) ) * 0x1b))
#define AES_KEYLEN 16
#define AES_keyExpSize 176
#define MSG_LEN 1024

typedef uint8_t state_t[4][4];

#ifndef HOST
struct AES_ctx
{
  uint8_t RoundKey[AES_keyExpSize];
  uint8_t Iv[AES_BLOCKLEN];
};
#endif

using message_t = std::array<uint8_t, MSG_LEN>;
using aes_key_t     = std::array<uint8_t, AES_KEYLEN>;

using ctx_vector_t     = datastructure::vector<AES_ctx, 1>;
using message_vector_t = datastructure::vector<message_t, 1>;

#define NUM_TILES (bsg_tiles_X*bsg_tiles_Y*bsg_pods_X*bsg_pods_Y)
