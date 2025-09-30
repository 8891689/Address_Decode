/* sha256.c */
/* Apache License, Version 2.0
   Copyright [2025] [8891689]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
   Author: 8891689 (https://github.com/8891689)
*/

#include "sha256.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef __builtin_bswap64
#define __builtin_bswap64(x) (((x) >> 56) | (((x) & 0x00ff000000000000ull) >> 40) | (((x) & 0x0000ff0000000000ull) >> 24) | (((x) & 0x000000ff00000000ull) >> 8) | (((x) & 0x00000000ff000000ull) << 8) | (((x) & 0x0000000000ff0000ull) << 24) | (((x) & 0x000000000000ff00ull) << 40) | ((x) << 56))
#endif

/******************************************************************************
 *                              基礎 C 語言實現                              *
 ******************************************************************************/

#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define CH(x,y,z)  (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x)     (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x)     (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x)    (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x)    (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

static const uint32_t k256[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};


static void sha256_transform(SHA256_CTX *ctx, const uint8_t data[]) {
    uint32_t m[64], a, b, c, d, e, f, g, h, t1, t2;
    int i, j;

    // 1. 消息排程 (Message Schedule)
    for (i = 0, j = 0; i < 16; ++i, j += 4) {
        m[i] = ((uint32_t)data[j] << 24) | ((uint32_t)data[j+1] << 16) | ((uint32_t)data[j+2] << 8) | ((uint32_t)data[j+3]);
    }
    for ( ; i < 64; ++i) {
        m[i] = m[i-16] + SIG0(m[i-15]) + m[i-7] + SIG1(m[i-2]);
    }
    
    // 2. 加載初始狀態
    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];
    
    // 3. 核心循環，使用宏完全展開 64 輪
    #define R(I) t1 = h + EP1(e) + CH(e,f,g) + k256[I] + m[I]; t2 = EP0(a) + MAJ(a,b,c); h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    R( 0) R( 1) R( 2) R( 3) R( 4) R( 5) R( 6) R( 7)
    R( 8) R( 9) R(10) R(11) R(12) R(13) R(14) R(15)
    R(16) R(17) R(18) R(19) R(20) R(21) R(22) R(23)
    R(24) R(25) R(26) R(27) R(28) R(29) R(30) R(31)
    R(32) R(33) R(34) R(35) R(36) R(37) R(38) R(39)
    R(40) R(41) R(42) R(43) R(44) R(45) R(46) R(47)
    R(48) R(49) R(50) R(51) R(52) R(53) R(54) R(55)
    R(56) R(57) R(58) R(59) R(60) R(61) R(62) R(63)
    #undef R

    // 4. 更新最終狀態
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

void sha256_init(SHA256_CTX *ctx) {
    ctx->datalen = 0; ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667; ctx->state[1] = 0xbb67ae85; ctx->state[2] = 0x3c6ef372; ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f; ctx->state[5] = 0x9b05688c; ctx->state[6] = 0x1f83d9ab; ctx->state[7] = 0x5be0cd19;
}

void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

void sha256_final(SHA256_CTX *ctx, uint8_t hash[]) {
    uint32_t i;

    // 1. 在修改 ctx->datalen 之前，先計算出最終的總位元長度。
    uint64_t total_bitlen = ctx->bitlen + ((uint64_t)ctx->datalen * 8);

    // 2. 添加填充標記 0x80
    ctx->data[ctx->datalen] = 0x80;
    ctx->datalen++;

    // 3. 處理填充，正確應對邊界情況 (當前數據塊空間不足時)
    if (ctx->datalen > 56) {
        memset(ctx->data + ctx->datalen, 0, 64 - ctx->datalen);
        sha256_transform(ctx, ctx->data);
        // 清空一個新塊，準備寫入長度
        memset(ctx->data, 0, 56);
    } else {
        memset(ctx->data + ctx->datalen, 0, 56 - ctx->datalen);
    }

    // 4. 將總長度以大端序（Big-Endian）寫入最後 8 字節
    ctx->data[56] = (uint8_t)(total_bitlen >> 56);
    ctx->data[57] = (uint8_t)(total_bitlen >> 48);
    ctx->data[58] = (uint8_t)(total_bitlen >> 40);
    ctx->data[59] = (uint8_t)(total_bitlen >> 32);
    ctx->data[60] = (uint8_t)(total_bitlen >> 24);
    ctx->data[61] = (uint8_t)(total_bitlen >> 16);
    ctx->data[62] = (uint8_t)(total_bitlen >> 8);
    ctx->data[63] = (uint8_t)(total_bitlen);

    // 5. 處理最後一個數據塊
    sha256_transform(ctx, ctx->data);

    // 6. 將 state 寄存器中的結果轉換為大端序的 hash 字節數組
    for (i = 0; i < 8; ++i) {
        hash[i * 4 + 0] = (ctx->state[i] >> 24) & 0xff;
        hash[i * 4 + 1] = (ctx->state[i] >> 16) & 0xff;
        hash[i * 4 + 2] = (ctx->state[i] >> 8)  & 0xff;
        hash[i * 4 + 3] = (ctx->state[i] >> 0)  & 0xff;
    }
}

void sha256(const uint8_t *data, size_t len, uint8_t *hash) {
    SHA256_CTX ctx; 
    sha256_init(&ctx); 
    sha256_update(&ctx, data, len); 
    sha256_final(&ctx, hash);
}

