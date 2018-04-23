/******************************************************************************
*        
*     Open source
*        
*******************************************************************************
*  file name:          hmac_sha256.h
*  author:              Chen Hao
*  version:             1.00
*  file description:   hmac sha256 header file
*******************************************************************************
*  revision history:    date               version                  author
*
*  change summary:   2018-4-23             1.00                    Chen Hao
*
******************************************************************************/
#ifndef __HMAC_SHA256_H__
#define __HMAC_SHA256_H__
/******************************************************************************
* Include Files
******************************************************************************/
#include "basetype.h"
#include "sha256.h"

/******************************************************************************
* Macros
******************************************************************************/

/******************************************************************************
* Types
******************************************************************************/
typedef struct 
{
    sha256_ctx ctx_inside;
    sha256_ctx ctx_outside;

    /* for hmac_reinit */
    sha256_ctx ctx_inside_reinit;
    sha256_ctx ctx_outside_reinit;

    uint8 block_ipad[SHA256_BLOCK_SIZE];
    uint8 block_opad[SHA256_BLOCK_SIZE];
} hmac_sha256_ctx;

/******************************************************************************
* Extern functions
******************************************************************************/
void hmac_sha256_init(hmac_sha256_ctx *ctx, const uint8 *key, uint32 key_size);
void hmac_sha256_reinit(hmac_sha256_ctx *ctx);
void hmac_sha256_update(hmac_sha256_ctx *ctx, const uint8 *message, uint32 message_len);
void hmac_sha256_final(hmac_sha256_ctx *ctx, uint8 *mac, uint32 mac_size);
void hmac_sha256(const uint8 *key, uint32 key_size, const uint8 *message, uint32 message_len,
                                 uint8 *mac, uint8 mac_size);

#endif /* __HMAC_SHA256_H__ */

