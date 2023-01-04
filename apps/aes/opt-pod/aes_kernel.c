/*

This is an implementation of the AES algorithm, specifically ECB, CTR and CBC mode.
Block size can be chosen in aes.h - available choices are AES128, AES192, AES256.

The implementation is verified against the test vectors in:
  National Institute of Standards and Technology Special Publication 800-38A 2001 ED

ECB-AES128
----------

  plain-text:
    6bc1bee22e409f96e93d7e117393172a
    ae2d8a571e03ac9c9eb76fac45af8e51
    30c81c46a35ce411e5fbc1191a0a52ef
    f69f2445df4f9b17ad2b417be66c3710

  key:
    2b7e151628aed2a6abf7158809cf4f3c

  resulting cipher
    3ad77bb40d7a3660a89ecaf32466ef97 
    f5d3d58503b9699de785895a96fdbaaf 
    43b1cd7f598ece23881b00e3ed030688 
    7b0c785e27e8ad3f8223207104725dd4 


NOTE:   String length must be evenly divisible by 16byte (str_len % 16 == 0)
        You should pad the end of the string with zeros if this is not the case.
        For AES192/256 the key size is proportionally larger.

*/


/*****************************************************************************/
/* Includes:                                                                 */
/*****************************************************************************/
#include <string.h> // CBC mode, for memset
#include "aes_kernel.h"
#ifdef BSG_MANYCORE
#include <bsg_manycore.h>
#include <bsg_set_tile_x_y.h>
#endif

/*****************************************************************************/
/* Defines:                                                                  */
/*****************************************************************************/
// The number of columns comprising a state in AES. This is a constant in AES. Value=4
#define Nb 4

#if defined(AES256) && (AES256 == 1)
    #define Nk 8
    #define Nr 14
#elif defined(AES192) && (AES192 == 1)
    #define Nk 6
    #define Nr 12
#else
    #define Nk 4        // The number of 32 bit words in a key.
    #define Nr 10       // The number of rounds in AES Cipher.
#endif

// jcallan@github points out that declaring Multiply as a function 
// reduces code size considerably with the Keil ARM compiler.
// See this link for more information: https://github.com/kokke/tiny-AES-C/pull/3
#ifndef MULTIPLY_AS_A_FUNCTION
  #define MULTIPLY_AS_A_FUNCTION 0
#endif




/*****************************************************************************/
/* Private variables:                                                        */
/*****************************************************************************/
// state - array holding the intermediate results during decryption.
typedef uint8_t state_t[4][4];



// The lookup-tables are marked const so they can be placed in read-only storage instead of RAM
// The numbers below can be computed dynamically trading ROM for RAM - 
// This can be useful in (embedded) bootloader applications, where ROM is often limited.
//#ifdef BSG_MANYCORE_SBOX_LOCAL
const uint8_t sbox[256] __attribute__((section(".dmem")))= {
//#else
//static const uint8_t sbox[256] = {
//#endif
  //0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
  0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
  0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
  0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
  0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
  0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
  0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
  0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
  0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
  0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
  0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
  0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
  0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
  0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
  0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
  0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
  0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };



/*****************************************************************************/
/* Private functions:                                                        */
/*****************************************************************************/
/*
static uint8_t getSBoxValue(uint8_t num)
{
  return sbox[num];
}
*/
#define getSBoxValue(num) (sbox[(num)])


// This function adds the round key to state.
// The round key is added to the state by an XOR function.
__attribute__ ((always_inline))
static void AddRoundKey(uint8_t round, state_t* state, const uint8_t* RoundKey)
{
/*
  uint8_t i,j;
  for (i = 0; i < 4; ++i)
  {
    for (j = 0; j < 4; ++j)
    {
      (*state)[i][j] ^= RoundKey[(round * Nb * 4) + (i * Nb) + j];
    }
  }
*/
  uint32_t * state32 = (uint32_t *) state;
  uint32_t * RoundKey32 = (uint32_t *) (&RoundKey[round * Nb * 4]);
  uint32_t t0 = state32[0];
  uint32_t t1 = state32[1];
  uint32_t t2 = state32[2];
  uint32_t t3 = state32[3];
  uint32_t r0 = RoundKey32[0];
  uint32_t r1 = RoundKey32[1];
  uint32_t r2 = RoundKey32[2];
  uint32_t r3 = RoundKey32[3];
  asm volatile("" ::: "memory");
  state32[0] = t0 ^ r0;  
  state32[1] = t1 ^ r1;  
  state32[2] = t2 ^ r2;  
  state32[3] = t3 ^ r3;  
  asm volatile("" ::: "memory");

  //bsg_unroll(1)
/*
  for (int i = 0; i < 4; ++i)
  {
    state32[i] ^= RoundKey32[(round * Nb) + i];
  }
*/
}

// The SubBytes Function Substitutes the values in the
// state matrix with values in an S-box.
__attribute__ ((always_inline))
void SubBytes(uint8_t * state)
{
  // first two rows
  uint8_t t0 = state[0];
  uint8_t t1 = state[1];
  uint8_t t2 = state[2];
  uint8_t t3 = state[3];
  uint8_t t4 = state[4];
  uint8_t t5 = state[5];
  uint8_t t6 = state[6];
  uint8_t t7 = state[7];
  asm volatile("" ::: "memory");
  uint8_t s0 = sbox[t0];
  uint8_t s1 = sbox[t1];
  uint8_t s2 = sbox[t2];
  uint8_t s3 = sbox[t3];
  uint8_t s4 = sbox[t4];
  uint8_t s5 = sbox[t5];
  uint8_t s6 = sbox[t6];
  uint8_t s7 = sbox[t7];
  asm volatile("" ::: "memory");
  state[0] = s0;
  state[1] = s1;
  state[2] = s2;
  state[3] = s3;
  state[4] = s4;
  state[5] = s5;
  state[6] = s6;
  state[7] = s7;
  asm volatile("" ::: "memory");

  // next two rows
  t0 = state[8];
  t1 = state[9];
  t2 = state[10];
  t3 = state[11];
  t4 = state[12];
  t5 = state[13];
  t6 = state[14];
  t7 = state[15];
  asm volatile("" ::: "memory");
  s0 = sbox[t0];
  s1 = sbox[t1];
  s2 = sbox[t2];
  s3 = sbox[t3];
  s4 = sbox[t4];
  s5 = sbox[t5];
  s6 = sbox[t6];
  s7 = sbox[t7];
  asm volatile("" ::: "memory");
  state[8] = s0;
  state[9] = s1;
  state[10] = s2;
  state[11] = s3;
  state[12] = s4;
  state[13] = s5;
  state[14] = s6;
  state[15] = s7;
  asm volatile("" ::: "memory");
}

// The ShiftRows() function shifts the rows in the state to the left.
// Each row is shifted with different offset.
// Offset = Row number. So the first row is not shifted.
__attribute__ ((always_inline))
static void ShiftRows(uint8_t* state)
{
  uint8_t t1, t2, t3, t4;

  // Rotate first row 1 columns to left  
  t1 = state[1];
  t2 = state[5];
  t3 = state[9];
  t4 = state[13];
  asm volatile("" ::: "memory");
  state[1] = t2;
  state[5] = t3;
  state[9] = t4;
  state[13] = t1;
  asm volatile("" ::: "memory");

  // Rotate second row 2 columns to left  
  t1 = state[2];
  t2 = state[6];
  t3 = state[10];
  t4 = state[14];
  asm volatile("" ::: "memory");
  state[2] = t3;
  state[6] = t4;
  state[10] = t1;
  state[14] = t2;
  asm volatile("" ::: "memory");

  // Rotate third row 3 columns to left
  t1 = state[3];
  t2 = state[7];
  t3 = state[11];
  t4 = state[15];
  asm volatile("" ::: "memory");
  state[3] = t4;
  state[7] = t1;
  state[11] = t2;
  state[15] = t3;
  asm volatile("" ::: "memory");
}
/*
__attribute__ ((always_inline))
uint8_t xtime(uint8_t x)
{
  //return ((x<<1) ^ (((x>>7) & 1) * 0x1b));
  return ((x<<1) ^ (((x>>7) ) * 0x1b));
}
*/
#define xtime(x) ((x<<1) ^ (((x>>7) ) * 0x1b))

// MixColumns function mixes the columns of the state matrix
__attribute__ ((always_inline))
void MixColumns(uint8_t * state)
{
  uint8_t t0, t1, t2, t3;
  uint8_t tmp1, tmp2, tmp3;
  for (int i = 0; i < 4; i++) {
    t0 = state[(4*i)+0];
    t1 = state[(4*i)+1];
    t2 = state[(4*i)+2];
    t3 = state[(4*i)+3];
    asm volatile("" ::: "memory");
    tmp1 = t0 ^ t1 ^ t2 ^ t3;
    tmp3 = t0 ^ t1;
    tmp2 = xtime(tmp3);
    state[(4*i)+0] = t0 ^ tmp1 ^ tmp2;
    tmp3 = t1 ^ t2;
    tmp2 = xtime(tmp3);
    state[(4*i)+1] = t1 ^ tmp1 ^ tmp2;
    tmp3 = t2 ^ t3;
    tmp2 = xtime(tmp3);
    state[(4*i)+2] = t2 ^ tmp1 ^ tmp2;
    tmp3 = t3 ^ t0;
    tmp2 = xtime(tmp3);
    state[(4*i)+3] = t3 ^ tmp1 ^ tmp2;
    asm volatile("" ::: "memory");
  }
  /////////////////////////////////////////
/*
  uint8_t i;
  uint8_t Tmp, Tm, t;
  for (i = 0; i < 4; ++i)
  {  
    t   = (*state)[i][0];
    Tmp = (*state)[i][0] ^ (*state)[i][1] ^ (*state)[i][2] ^ (*state)[i][3] ;
    Tm  = (*state)[i][0] ^ (*state)[i][1] ; Tm = xtime(Tm);  (*state)[i][0] ^= Tm ^ Tmp ;
    Tm  = (*state)[i][1] ^ (*state)[i][2] ; Tm = xtime(Tm);  (*state)[i][1] ^= Tm ^ Tmp ;
    Tm  = (*state)[i][2] ^ (*state)[i][3] ; Tm = xtime(Tm);  (*state)[i][2] ^= Tm ^ Tmp ;
    Tm  = (*state)[i][3] ^ t ;              Tm = xtime(Tm);  (*state)[i][3] ^= Tm ^ Tmp ;
  }
*/
}



// Cipher is the main function that encrypts the PlainText.
void Cipher(state_t* state, const uint8_t* RoundKey)
{
  uint8_t round = 0;

  // Add the First round key to the state before starting the rounds.
  AddRoundKey(0, state, RoundKey);

  // There will be Nr rounds.
  // The first Nr-1 rounds are identical.
  // These Nr rounds are executed in the loop below.
  // Last one without MixColumns()
  //bsg_unroll(1)
  for (round = 1; ; ++round)
  {
    SubBytes((uint8_t *) state);
    ShiftRows((uint8_t *) state);
    if (round == Nr) {
      break;
    }
    MixColumns((uint8_t *) state);
    AddRoundKey(round, state, RoundKey);
  }
  // Add round key to last round
  AddRoundKey(Nr, state, RoundKey);
}


/*****************************************************************************/
/* Public functions:                                                         */
/*****************************************************************************/



#if defined(CBC) && (CBC == 1)

/*
static void XorWithIv(uint8_t* buf, const uint8_t* Iv)
{
  uint8_t i;
  for (i = 0; i < AES_BLOCKLEN; ++i) // The block in AES is always 128bit no matter the key size
  {
    buf[i] ^= Iv[i];
  }
}
*/


#ifdef BSG_MANYCORE_OPTIMIZED
/*
void inline alignmemcpy(uint32_t * restrict dest, uint32_t * restrict src, size_t len){
        const unsigned int unroll = 4;
        uint32_t * tail = (src + (len/sizeof(src[0])));
        while(src < tail){
                bsg_unroll(8)
                for(int i =0 ; i < unroll; ++i){
                        dest[i] = src[i];
                }
                src += unroll;
                dest += unroll;
        }
}
*/

uint32_t localbuf0[AES_BLOCKLEN/sizeof(uint32_t)];
uint32_t localbuf1[AES_BLOCKLEN/sizeof(uint32_t)];
uint32_t localRoundKey[AES_keyExpSize/sizeof(uint32_t)];

void AES_CBC_encrypt_buffer(struct AES_ctx *ctx, uint8_t * restrict buf, size_t length)
{
  uint32_t *dbuf = (uint32_t *) &buf[0];
  uint32_t *pbuf = &localbuf0[0];
  uint32_t *pIv  = &localbuf1[0];
  uint32_t *temp;

  //bsg_lw_prefetch(ctx->RoundKey);
  //bsg_lw_prefetch(ctx->Iv);
  //bsg_lw_prefetch((ctx->Iv + AES_BLOCKLEN));
  //bsg_lw_prefetch((ctx->Iv + AES_BLOCKLEN * 2));
  //bsg_lw_prefetch((ctx->Iv + AES_BLOCKLEN * 3));

  //bsg_fence();
  
  //memcpy(localRoundKey, ctx->RoundKey, AES_keyExpSize);
  //memcpy(pIv, ctx->Iv, AES_BLOCKLEN);

  // load roundkey
  uint32_t *dramRoundKey = (uint32_t *) &ctx->RoundKey[0];
  bsg_unroll(1)
  for (int i = 0; i < AES_keyExpSize/sizeof(uint32_t); i+=8) {
    uint32_t r0 = dramRoundKey[i+0];
    uint32_t r1 = dramRoundKey[i+1];
    uint32_t r2 = dramRoundKey[i+2];
    uint32_t r3 = dramRoundKey[i+3];
    uint32_t r4 = dramRoundKey[i+4];
    uint32_t r5 = dramRoundKey[i+5];
    uint32_t r6 = dramRoundKey[i+6];
    uint32_t r7 = dramRoundKey[i+7];
    asm volatile("" ::: "memory");
    localRoundKey[i+0] = r0;
    localRoundKey[i+1] = r1;
    localRoundKey[i+2] = r2;
    localRoundKey[i+3] = r3;
    localRoundKey[i+4] = r4;
    localRoundKey[i+5] = r5;
    localRoundKey[i+6] = r6;
    localRoundKey[i+7] = r7;
  }

  // load Iv
  uint32_t *dramIv = (uint32_t *) &ctx->Iv[0];
  uint32_t Iv0 = dramIv[0];
  uint32_t Iv1 = dramIv[1];
  uint32_t Iv2 = dramIv[2];
  uint32_t Iv3 = dramIv[3];
  asm volatile("" ::: "memory");
  pIv[0] = Iv0;
  pIv[1] = Iv1;
  pIv[2] = Iv2;
  pIv[3] = Iv3;

  bsg_unroll(1)
  for (size_t i = 0; i < length; i += AES_BLOCKLEN)
  {
    // Load and XorWithIv
    uint32_t w0 = dbuf[0];
    uint32_t w1 = dbuf[1];
    uint32_t w2 = dbuf[2];
    uint32_t w3 = dbuf[3];
    pbuf[0] = w0 ^ pIv[0];
    pbuf[1] = w1 ^ pIv[1];
    pbuf[2] = w2 ^ pIv[2];
    pbuf[3] = w3 ^ pIv[3];
    //alignmemcpy(pbuf, buf, AES_BLOCKLEN);
    //bsg_lw_prefetch((buf + AES_BLOCKLEN));
    //XorWithIv(pbuf, pIv);

    // cipher
    Cipher((state_t*)pbuf, (const uint8_t *) localRoundKey);

    // copy encrypted result to dram.
    dbuf[0] = pbuf[0]; 
    dbuf[1] = pbuf[1]; 
    dbuf[2] = pbuf[2]; 
    dbuf[3] = pbuf[3]; 
    //alignmemcpy(buf, pbuf, AES_BLOCKLEN);

    // swap pointers
    temp = pIv;
    pIv = pbuf;
    pbuf = temp;

    // Increment block
    dbuf += AES_BLOCKLEN/sizeof(uint32_t);
  }
  /* store Iv in ctx for next call */
  Iv0 = pIv[0];
  Iv1 = pIv[1];
  Iv2 = pIv[2];
  Iv3 = pIv[3];
  asm volatile("" ::: "memory");
  dramIv[0] = Iv0;
  dramIv[1] = Iv1;
  dramIv[2] = Iv2;
  dramIv[3] = Iv3;
  //memcpy(ctx->Iv, pIv, AES_BLOCKLEN);
  //memcpy(ctx->RoundKey, localRoundKey, AES_keyExpSize);
}
#else
void AES_CBC_encrypt_buffer(struct AES_ctx *ctx, uint8_t* buf, size_t length)
{
  size_t i;
  uint8_t *Iv = ctx->Iv;
  for (i = 0; i < length; i += AES_BLOCKLEN)
  {
    XorWithIv(buf, Iv);
    Cipher((state_t*)buf, ctx->RoundKey);
    Iv = buf;
    buf += AES_BLOCKLEN;
  }
  /* store Iv in ctx for next call */
  memcpy(ctx->Iv, Iv, AES_BLOCKLEN);
}
#endif

#endif // #if defined(CBC) && (CBC == 1)



