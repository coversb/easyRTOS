/******************************************************************************
*
*     Open source
*
*******************************************************************************
*  file name:          aes.c
*  author:              Chen Hao
*  version:             1.00
*  file description:   aes ecb pkcs5/7 padding
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:   2018-4-23      1.00                    Chen Hao
*
******************************************************************************/
/******************************************************************************
* Include Files
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "aes.h"

#define XTIME(x) ((x<<1) ^ (((x>>7) & 1) * 0x1b))
#define MULTIPLY(x,y)  \
(((y & 1) * x) ^ ((y>>1 & 1) * XTIME(x)) ^ ((y>>2 & 1) * XTIME(XTIME(x))) ^ ((y>>3 & 1) * XTIME(XTIME(XTIME(x)))) ^ ((y>>4 & 1) * XTIME(XTIME(XTIME(XTIME(x))))))

/******************************************************************************
* Variables (Extern, Global and Static)
******************************************************************************/
//temp buffer
static uint8 inBuff[4 * Nb];
static uint8 outBuff[4 * Nb];
// 4x4 state matrix
static uint8 stateMat[4][Nb];
//Expansion key
static uint8 roundKey[4 * Nb * (Nr + 1)];
//AES key
static uint8 encryptKey[4 * Nk];

static const uint8 RCON[255] = {
    0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a,
    0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39,
    0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a,
    0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8,
    0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef,
    0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc,
    0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b,
    0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3,
    0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94,
    0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20,
    0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35,
    0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f,
    0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04,
    0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63,
    0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd,
    0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb
};

static const uint8 SBOX[256] =
{
//0     1       2       3       4       5       6       7       8       9       A       B       C       D       E       F
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
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

static const uint8 RSBOX[256] =
{
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

/******************************************************************************
* Local Functions
******************************************************************************/
/******************************************************************************
* Function    : aes_key_expansion
*
* Author      : Chen Hao
*
* Parameters  :
*
* Return      :
*
* Description :
******************************************************************************/
static void aes_key_expansion(void)
{
    uint32 i, j;
    uint8 temp[4];
    uint8 k = 0;

    //第一轮使用原始密钥
    for(i = 0; i < Nk; ++i)
    {
        roundKey[i * 4] = encryptKey[i * 4];
        roundKey[i * 4 + 1] = encryptKey[i * 4 + 1];
        roundKey[i * 4 + 2] = encryptKey[i * 4 + 2];
        roundKey[i * 4 + 3] = encryptKey[i * 4 + 3];
    }

    //其他轮次密钥来自上轮密钥
    while (i < (Nb * (Nr + 1)))
    {
        for(j = 0; j < 4; ++j)
        {
            temp[j] = roundKey[(i - 1) * 4 + j];
        }
        if( i % Nk == 0 )
        {
            //RotWord
            //make [a0,a1,a2,a3] to [a1,a2,a3,a0]
            {
                k = temp[0];
                temp[0] = temp[1];
                temp[1] = temp[2];
                temp[2] = temp[3];
                temp[3] = k;
            }
            //SubWord, convert by S box
            {
                temp[0] = SBOX[temp[0]];
                temp[1] = SBOX[temp[1]];
                temp[2] = SBOX[temp[2]];
                temp[3] = SBOX[temp[3]];
            }
            temp[0] = temp[0] ^ RCON[i / Nk];

        }
        else if ((Nk > 6) && (i % Nk == 4))
        {
            //! ????SubWord??
            temp[0] = SBOX[temp[0]];
            temp[1] = SBOX[temp[1]];
            temp[2] = SBOX[temp[2]];
            temp[3] = SBOX[temp[3]];
        }
        roundKey[i * 4 + 0] = roundKey[(i - Nk) * 4 + 0] ^ temp[0];
        roundKey[i * 4 + 1] = roundKey[(i - Nk) * 4 + 1] ^ temp[1];
        roundKey[i * 4 + 2] = roundKey[(i - Nk) * 4 + 2] ^ temp[2];
        roundKey[i * 4 + 3] = roundKey[(i - Nk) * 4 + 3] ^ temp[3];
        i++;
    }
}

/******************************************************************************
* Function    : aes_add_round_key
*
* Author      : Chen Hao
*
* Parameters  :
*
* Return      :
*
* Description :
******************************************************************************/
static void aes_add_round_key(uint32 round)
{
    uint32 i, j;
    for (i = 0; i < Nb; ++i)
    {
        for (j = 0; j < 4; ++j)
        {
            stateMat[j][i] ^= roundKey[round * Nb * 4 + i * Nb + j];
        }
    }
}

//encrypt by AES-256
/******************************************************************************
* Function    : aes_sub_bytes
*
* Author      : Chen Hao
*
* Parameters  :
*
* Return      :
*
* Description :
******************************************************************************/
static void aes_sub_bytes(void)
{
    for (uint8 i = 0; i < 4; ++i)
    {
        for(uint8 j = 0; j < Nb; ++j)
        {
            stateMat[i][j] = SBOX[stateMat[i][j]];
        }
    }
}

/******************************************************************************
* Function    : aes_shift_rows
*
* Author      : Chen Hao
*
* Parameters  :
*
* Return      :
*
* Description :
******************************************************************************/
static void aes_shift_rows(void)
{
    uint8 temp;

    //first row left move 1 column
    temp = stateMat[1][0];
    stateMat[1][0] = stateMat[1][1];
    stateMat[1][1] = stateMat[1][2];
    stateMat[1][2] = stateMat[1][3];
    stateMat[1][3] = temp;

    //second row left move 2 column
    temp = stateMat[2][0];
    stateMat[2][0] = stateMat[2][2];
    stateMat[2][2] = temp;

    temp = stateMat[2][1];
    stateMat[2][1] = stateMat[2][3];
    stateMat[2][3] = temp;

    //third row left move 3 column
    temp = stateMat[3][0];
    stateMat[3][0] = stateMat[3][3];
    stateMat[3][3] = stateMat[3][2];
    stateMat[3][2] = stateMat[3][1];
    stateMat[3][1] = temp;
}

/******************************************************************************
* Function    : aes_mix_columns
*
* Author      : Chen Hao
*
* Parameters  :
*
* Return      :
*
* Description :
******************************************************************************/
static void aes_mix_columns(void)
{
    uint8 Tmp, Tm, t;
    for (uint8 i = 0; i < Nb; ++i)
    {
        t = stateMat[0][i];
        Tmp = stateMat[0][i] ^ stateMat[1][i] ^ stateMat[2][i] ^ stateMat[3][i];
        Tm = stateMat[0][i] ^ stateMat[1][i];
        Tm = XTIME(Tm);
        stateMat[0][i] ^= Tm ^ Tmp;
        Tm = stateMat[1][i] ^ stateMat[2][i];
        Tm = XTIME(Tm);
        stateMat[1][i] ^= Tm ^ Tmp;
        Tm = stateMat[2][i] ^ stateMat[3][i];
        Tm = XTIME(Tm);
        stateMat[2][i] ^= Tm ^ Tmp;
        Tm = stateMat[3][i] ^ t;
        Tm = XTIME(Tm);
        stateMat[3][i] ^= Tm ^ Tmp;
    }
}

/******************************************************************************
* Function    : aes_cipher
*
* Author      : Chen Hao
*
* Parameters  :
*
* Return      :
*
* Description :
******************************************************************************/
static void aes_cipher(void)
{
    uint32 i = 0;
    uint32 j = 0;
    uint32 round = 0;

    //set state mat
    for (i = 0; i < Nb; ++i)
    {
        for (j = 0; j < 4; ++j)
        {
            stateMat[j][i] = inBuff[i * 4 + j];
        }
    }

    //Add first round sub key to state mat
    aes_add_round_key(0);

    //9 round iteration
    for (round = 1; round < Nr; ++round)
    {
        aes_sub_bytes();
        aes_shift_rows();
        aes_mix_columns();
        aes_add_round_key(round);
    }

    //last round iteration
    {
        aes_sub_bytes();
        aes_shift_rows();
        aes_add_round_key(Nr);
    }

    //copy result to out buffer
    for (i = 0; i < 4; ++i)
    {
        for (j = 0; j < 4; ++j)
        {
            outBuff[i * 4 + j] = stateMat[j][i];
        }
    }
}

/******************************************************************************
* Function    : aes_encrypt_block
*
* Author      : Chen Hao
*
* Parameters  :
*
* Return      :
*
* Description :
******************************************************************************/
static void aes_encrypt_block(uint8 inData[4 * Nb], uint8 outData[4 * Nb], uint8 key[4 * Nk])
{
    memset(encryptKey, 0, sizeof(encryptKey));
    memset(inBuff, 0, sizeof(inBuff));

    for (uint8 i = 0; i < Nk * 4; ++i)
    {
        encryptKey[i] = key[i];
    }
    for (uint8 i = 0; i < Nb * 4; ++i)
    {
        inBuff[i] = inData[i];
    }

    //密钥膨胀
    aes_key_expansion();

    //迭代加密
    aes_cipher();
    memcpy(outData, outBuff, 4 * Nb);
}

//decrypt by AES-256
/******************************************************************************
* Function    : aes_inv_sub_bytes
*
* Author      : Chen Hao
*
* Parameters  :
*
* Return      :
*
* Description :
******************************************************************************/
static void aes_inv_sub_bytes(void)
{
    for (uint8 i = 0; i < 4; ++i)
    {
        for (uint8 j = 0; j < Nb; ++j)
        {
            stateMat[i][j] = RSBOX[stateMat[i][j]];
        }
    }
}

/******************************************************************************
* Function    : aes_inv_shift_rows
*
* Author      : Chen Hao
*
* Parameters  :
*
* Return      :
*
* Description :
******************************************************************************/
static void aes_inv_shift_rows(void)
{
    unsigned char temp;

    //first row right move 1 column
    temp = stateMat[1][3];
    stateMat[1][3] = stateMat[1][2];
    stateMat[1][2] = stateMat[1][1];
    stateMat[1][1] = stateMat[1][0];
    stateMat[1][0] = temp;

    //second row right move 2 column
    temp = stateMat[2][0];
    stateMat[2][0] = stateMat[2][2];
    stateMat[2][2] = temp;

    temp = stateMat[2][1];
    stateMat[2][1] = stateMat[2][3];
    stateMat[2][3] = temp;

    //third row right move 3 column
    temp = stateMat[3][0];
    stateMat[3][0] = stateMat[3][1];
    stateMat[3][1] = stateMat[3][2];
    stateMat[3][2] = stateMat[3][3];
    stateMat[3][3] = temp;
}

/******************************************************************************
* Function    : aes_inv_mix_columns
*
* Author      : Chen Hao
*
* Parameters  :
*
* Return      :
*
* Description :
******************************************************************************/
static void aes_inv_mix_columns(void)
{
    uint8 a, b, c, d;
    for (uint8 i = 0; i < Nb; ++i)
    {
        a = stateMat[0][i];
        b = stateMat[1][i];
        c = stateMat[2][i];
        d = stateMat[3][i];

        stateMat[0][i] = MULTIPLY(a, 0x0e) ^ MULTIPLY(b, 0x0b) ^ MULTIPLY(c, 0x0d) ^ MULTIPLY(d, 0x09);
        stateMat[1][i] = MULTIPLY(a, 0x09) ^ MULTIPLY(b, 0x0e) ^ MULTIPLY(c, 0x0b) ^ MULTIPLY(d, 0x0d);
        stateMat[2][i] = MULTIPLY(a, 0x0d) ^ MULTIPLY(b, 0x09) ^ MULTIPLY(c, 0x0e) ^ MULTIPLY(d, 0x0b);
        stateMat[3][i] = MULTIPLY(a, 0x0b) ^ MULTIPLY(b, 0x0d) ^ MULTIPLY(c, 0x09) ^ MULTIPLY(d, 0x0e);
    }
}

/******************************************************************************
* Function    : aes_inv_cipher
*
* Author      : Chen Hao
*
* Parameters  :
*
* Return      :
*
* Description :
******************************************************************************/
static void aes_inv_cipher(void)
{
    uint32 i = 0;
    uint32 j = 0;
    uint32 round = 0;

    //set state mat
    for (i = 0; i < Nb; ++i)
    {
        for (j = 0; j < 4; ++j)
        {
            stateMat[j][i] = inBuff[i * 4 + j];
        }
    }

    //Add first round sub key to state mat
    aes_add_round_key(Nr);

    //9 round iteration
    for (round = Nr - 1; round > 0; --round)
    {
        aes_inv_shift_rows();
        aes_inv_sub_bytes();
        aes_add_round_key(round);
        aes_inv_mix_columns();
    }

    //last round iteration
    aes_inv_shift_rows();
    aes_inv_sub_bytes();
    aes_add_round_key(0);

    //copy result to out buffer
    for (i = 0; i < Nb; ++i)
    {
        for (j = 0; j < 4; ++j)
        {
            outBuff[i * 4 + j] = stateMat[j][i];
        }
    }
}

/******************************************************************************
* Function    : aes_decrypt_block
*
* Author      : Chen Hao
*
* Parameters  :
*
* Return      :
*
* Description :
******************************************************************************/
static void aes_decrypt_block(uint8 inData[4 * Nb], uint8 outData[4 * Nb], uint8 key[4 * Nk])
{
    uint8 i = 0;

    for (i = 0; i < Nk * 4; ++i)
    {
        encryptKey[i] = key[i];
    }
    for (i = 0; i < Nb * 4; ++i)
    {
        inBuff[i] = inData[i];
    }

    //密钥膨胀
    aes_key_expansion();

    //迭代解密
    aes_inv_cipher();
    memcpy(outData, outBuff , 4 * Nb);
}

/******************************************************************************
* Function    : aes_encrypt
*
* Author      : Chen Hao
*
* Parameters  :
*
* Return      :
*
* Description : AES-256 encrypt
******************************************************************************/
uint16 aes_encrypt(void *inData, uint16 inLen, void *keyData, void *outData)
{
    uint8 *in = (uint8*)inData;
    uint8 *out = (uint8*)outData;
    uint8 *key = (uint8*)keyData;

    uint8  encryptData[4 * Nb];
    uint32 encryptCnt = inLen / (Nb * 4);
    uint32 leftByte = inLen % (Nb * 4);

    if (encryptCnt > 0)
    {
        //each 16 bytes do a encrypt
        for (uint8 index = 0; index < encryptCnt; ++index)
        {
            memcpy(&encryptData[0], in + index * (Nb * 4), Nb * 4);
            aes_encrypt_block(encryptData, out + index * (4 * Nb), key);
        }
    }

    //fill left data to 16 bytes, pading
    if (leftByte != 0)
    {
        memcpy(encryptData, in + encryptCnt * (4 * Nb), leftByte);
        memset(&encryptData[leftByte], (4 * Nb - leftByte), (4 * Nb - leftByte)); //?0x00??4*Nb?byte
    }
    else
    {
        memset(encryptData, 4 * Nb, 4 * Nb);
    }
    aes_encrypt_block(encryptData, out + encryptCnt * (4 * Nb), key);

    return (encryptCnt + 1) * (4 * Nb);
}

/******************************************************************************
* Function    : aes_decrypt
*
* Author      : Chen Hao
*
* Parameters  :
*
* Return      :
*
* Description : AES-256 decrypt
******************************************************************************/
uint16 aes_decrypt(void *inData, uint16 inLen, void *keyData, void *outData)
{
    uint8 *in = (uint8*)inData;
    uint8 *out = (uint8*)outData;
    uint8 *key = (uint8*)keyData;
    uint8  decryptData[4 * Nb];
    uint32 decryptCnt = inLen / (Nb * 4);

    if (inLen % (4 * Nb) != 0)
    {
        return  0;
    }

    for (uint8 index = 0; index < decryptCnt; ++index)
    {
        memcpy(decryptData, in + index * (4 * Nb), Nb * 4);
        aes_decrypt_block(decryptData, out + index * (4 * Nb), key);
    }

    //delete pading data
    uint8 padLen = ((uint8*)outData)[inLen - 1];
    bool hasPadding = true;
    for (int i = inLen - 1; i > inLen - padLen; --i)
    {
        if (((uint8*)outData)[i] != padLen)
        {
            hasPadding = false;
            break;
        }
    }

    if (hasPadding)
    {
        return inLen - padLen;
    }
    else
    {
        return inLen;
    }
}

