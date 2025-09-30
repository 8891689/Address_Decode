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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

#include "bech32.h"

/* Bech32 字符集 */
static const char CHARSET[] = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

/* 预计算的字符映射表 */
static uint8_t CHAR_MAP[256] = {0};

/* 初始化字符映射表 */
static void init_char_map() {
    static int initialized = 0;
    if (initialized) return;
    
    for (int i = 0; i < 32; i++) {
        CHAR_MAP[(unsigned char)CHARSET[i]] = i;
    }
    initialized = 1;
}

/* --- 内部函数 --- */

/* 多项式计算 - 使用循环展开和预计算 */
static inline uint32_t bech32_polymod(const uint8_t *values, size_t values_len) {
    uint32_t chk = 1;
    size_t i = 0;
    
    // 预计算的生成多项式
    static const uint32_t GEN[5] = {
        0x3b6a57b2, 0x26508e6d, 0x1ea119fa, 0x3d4233dd, 0x2a1462b3
    };
    
    // 处理4字节块
    for (; i + 4 <= values_len; i += 4) {
        uint8_t v0 = values[i];
        uint8_t v1 = values[i+1];
        uint8_t v2 = values[i+2];
        uint8_t v3 = values[i+3];
        
        // 展开内部循环
        uint8_t top = chk >> 25;
        chk = ((chk & 0x1ffffff) << 5) | v0;
        for (int j = 0; j < 5; j++) {
            if (top & (1 << j)) chk ^= GEN[j];
        }
        
        top = chk >> 25;
        chk = ((chk & 0x1ffffff) << 5) | v1;
        for (int j = 0; j < 5; j++) {
            if (top & (1 << j)) chk ^= GEN[j];
        }
        
        top = chk >> 25;
        chk = ((chk & 0x1ffffff) << 5) | v2;
        for (int j = 0; j < 5; j++) {
            if (top & (1 << j)) chk ^= GEN[j];
        }
        
        top = chk >> 25;
        chk = ((chk & 0x1ffffff) << 5) | v3;
        for (int j = 0; j < 5; j++) {
            if (top & (1 << j)) chk ^= GEN[j];
        }
    }
    
    // 处理剩余字节
    for (; i < values_len; i++) {
        uint8_t top = chk >> 25;
        chk = ((chk & 0x1ffffff) << 5) | values[i];
        for (int j = 0; j < 5; j++) {
            if (top & (1 << j)) chk ^= GEN[j];
        }
    }
    
    return chk;
}

/* 将 HRP 扩展为校验和计算用的数组  */
static inline void bech32_hrp_expand(const char *hrp, uint8_t *output) {
    size_t hrp_len = strlen(hrp);
    for (size_t i = 0; i < hrp_len; i++) {
        output[i] = hrp[i] >> 5;
    }
    output[hrp_len] = 0;
    for (size_t i = 0; i < hrp_len; i++) {
        output[hrp_len + 1 + i] = hrp[i] & 31;
    }
}

/* 超快速校验和验证 - 优化BECH32M */
static inline int bech32_fast_verify_checksum(const char *hrp, const uint8_t *data, 
                                            size_t data_len, int *out_version) {
    size_t hrp_len = strlen(hrp);
    
    // 内联HRP扩展计算，避免函数调用
    uint8_t hrp_expanded[128];
    for (size_t i = 0; i < hrp_len; i++) {
        hrp_expanded[i] = hrp[i] >> 5;
    }
    hrp_expanded[hrp_len] = 0;
    for (size_t i = 0; i < hrp_len; i++) {
        hrp_expanded[hrp_len + 1 + i] = hrp[i] & 31;
    }
    
    size_t total_len = 2 * hrp_len + 1 + data_len;
    if (total_len > 256) return 0;
    
    // 使用栈上数组，避免动态分配
    uint8_t values[256];
    memcpy(values, hrp_expanded, 2 * hrp_len + 1);
    memcpy(values + 2 * hrp_len + 1, data, data_len);
    
    uint32_t chk = bech32_polymod(values, total_len);
    
    // 同时检查两个版本，避免重复计算
    if (chk == 1) {
        *out_version = 0;
        return 1;
    } else if (chk == 0x2bc830a3) {
        *out_version = 1;
        return 1;
    }
    
    return 0;
}

/* 根据 HRP 与数据生成 6 个校验值 - 优化 */
static inline void bech32_create_checksum(const char *hrp, const uint8_t *data, 
                                        size_t data_len, uint8_t *checksum, int version) {
    size_t hrp_len = strlen(hrp);
    uint8_t hrp_expanded[128];
    bech32_hrp_expand(hrp, hrp_expanded);
    
    size_t total_len = 2 * hrp_len + 1 + data_len + 6;
    uint8_t values[256];
    memcpy(values, hrp_expanded, 2 * hrp_len + 1);
    memcpy(values + 2 * hrp_len + 1, data, data_len);
    memset(values + 2 * hrp_len + 1 + data_len, 0, 6);
    
    uint32_t polymod = bech32_polymod(values, total_len);
    uint32_t constant = (version == 0) ? 1 : 0x2bc830a3;
    polymod ^= constant;
    
    // 展开循环
    checksum[0] = (polymod >> 25) & 31;
    checksum[1] = (polymod >> 20) & 31;
    checksum[2] = (polymod >> 15) & 31;
    checksum[3] = (polymod >> 10) & 31;
    checksum[4] = (polymod >> 5) & 31;
    checksum[5] = polymod & 31;
}

/* 根据 HRP 与数据生成 Bech32 字符串 - 优化 */
static char *bech32_encode(const char *hrp, const uint8_t *data, size_t data_len, int version) {
    uint8_t checksum[6];
    bech32_create_checksum(hrp, data, data_len, checksum, version);
    
    size_t hrp_len = strlen(hrp);
    size_t output_len = hrp_len + 1 + data_len + 6;
    char *ret = malloc(output_len + 1);
    if (!ret) return NULL;
    
    // 复制HRP
    memcpy(ret, hrp, hrp_len);
    ret[hrp_len] = '1';
    
    // 复制数据部分
    for (size_t i = 0; i < data_len; i++) {
        ret[hrp_len + 1 + i] = CHARSET[data[i]];
    }
    
    // 复制校验和
    for (size_t i = 0; i < 6; i++) {
        ret[hrp_len + 1 + data_len + i] = CHARSET[checksum[i]];
    }
    
    ret[output_len] = '\0';
    return ret;
}

/* 解码 Bech32 字符串 - 优化BECH32M性能 */
static int bech32_decode_impl(const char *bech, char *out_hrp, uint8_t *out_data, 
                             size_t *out_data_len, int *out_version) {
    size_t len = strlen(bech);
    if (len < 8 || len > 90) return 0;
    
    // 初始化字符映射表
    init_char_map();
    
    // 查找分隔符位置
    int pos = -1;
    for (size_t i = 0; i < len; i++) {
        if (bech[i] == '1') {
            pos = i;
            break;
        }
    }
    if (pos < 1 || pos + 7 > (int)len) return 0;
    
    // 提取HRP
    size_t hrp_len = pos;
    memcpy(out_hrp, bech, hrp_len);
    out_hrp[hrp_len] = '\0';
    
    // 转换数据部分
    size_t data_part_len = len - pos - 1;
    if (data_part_len < 6) return 0;
    
    // 快速转换数据部分
    for (size_t i = 0; i < data_part_len; i++) {
        uint8_t c = (uint8_t)bech[pos + 1 + i];
        out_data[i] = CHAR_MAP[c];
        if (out_data[i] == 0 && c != 'q') return 0; // 无效字符
    }
    
    // 使用快速校验和验证（同时检查两个版本）
    if (!bech32_fast_verify_checksum(out_hrp, out_data, data_part_len, out_version)) {
        return 0;
    }
    
    *out_data_len = data_part_len - 6;
    return 1;
}

/* 位转换函数 - 优化 */
static inline int convertbits(const uint8_t *in, size_t inlen, int frombits, int tobits, 
                            int pad, uint8_t *out, size_t *outlen) {
    uint32_t acc = 0;
    int bits = 0;
    size_t out_idx = 0;
    uint32_t maxv = (1 << tobits) - 1;
    
    // 处理8位到5位转换的常见情况
    if (frombits == 8 && tobits == 5) {
        for (size_t i = 0; i < inlen; i++) {
            acc = (acc << 8) | in[i];
            bits += 8;
            while (bits >= 5) {
                bits -= 5;
                out[out_idx++] = (acc >> bits) & 31;
            }
        }
    } else if (frombits == 5 && tobits == 8) {
        for (size_t i = 0; i < inlen; i++) {
            acc = (acc << 5) | in[i];
            bits += 5;
            while (bits >= 8) {
                bits -= 8;
                out[out_idx++] = (acc >> bits) & 255;
            }
        }
    } else {
        // 通用情况
        for (size_t i = 0; i < inlen; i++) {
            acc = (acc << frombits) | in[i];
            bits += frombits;
            while (bits >= tobits) {
                bits -= tobits;
                out[out_idx++] = (acc >> bits) & maxv;
            }
        }
    }
    
    if (pad && bits > 0) {
        out[out_idx++] = (acc << (tobits - bits)) & maxv;
    }
    
    *outlen = out_idx;
    return 1;
}

/* 内部实现：解码 segwit 地址 - 优化BECH32M */
static int segwit_addr_decode_internal(const char *addr, const char *hrp, int *witver, 
                                      uint8_t *witprog, size_t *witprog_len) {
    char hrp_decoded[84];
    uint8_t data[90];
    size_t data_len;
    int version;
    
    if (!bech32_decode_impl(addr, hrp_decoded, data, &data_len, &version)) {
        return 0;
    }
    
    if (strcmp(hrp_decoded, hrp) != 0) {
        return 0;
    }
    
    if (data_len < 1) {
        return 0;
    }
    
    *witver = data[0];
    
    // 检查版本是否一致
    if (*witver != version) {
        return 0;
    }
    
    uint8_t conv[40];
    size_t conv_len;
    if (!convertbits(data + 1, data_len - 1, 5, 8, 0, conv, &conv_len)) {
        return 0;
    }
    
    if (conv_len < 2 || conv_len > 40) {
        return 0;
    }
    
    if (*witver > 16) {
        return 0;
    }
    
    if (*witver == 0 && conv_len != 20 && conv_len != 32) {
        return 0;
    }
    
    if (*witprog_len < conv_len) {
        return 0;
    }
    
    memcpy(witprog, conv, conv_len);
    *witprog_len = conv_len;
    return 1;
}

/* 内部实现：编码 segwit 地址 - 优化 */
static char *segwit_addr_encode_internal(const char *hrp, int witver, const uint8_t *witprog, 
                                       size_t witprog_len) {
    uint8_t five_bit[200];
    size_t five_bit_len;
    if (!convertbits(witprog, witprog_len, 8, 5, 1, five_bit, &five_bit_len)) {
        return NULL;
    }
    
    uint8_t data[200];
    data[0] = witver;
    memcpy(data + 1, five_bit, five_bit_len);
    size_t data_len = five_bit_len + 1;
    
    char *ret = bech32_encode(hrp, data, data_len, witver);
    return ret;
}

/* --- 对外接口 --- */

/* segwit_addr_encode: 将 witness 程序编码为 Bech32 格式地址 */
int segwit_addr_encode(char *output, const char *hrp, int witver, const uint8_t *witprog, size_t witprog_len) {
    char *encoded = segwit_addr_encode_internal(hrp, witver, witprog, witprog_len);
    if (!encoded) return 0;
    strcpy(output, encoded);
    free(encoded);
    return 1;
}

/* segwit_addr_decode: 解码 Bech32 格式的 segwit 地址 */
int segwit_addr_decode(const char *addr, const char *hrp, int *witver, uint8_t *witprog, size_t *witprog_len) {
    return segwit_addr_decode_internal(addr, hrp, witver, witprog, witprog_len);
}
