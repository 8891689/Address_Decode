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
#include "base58.h"
#include "sha256.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

/* 比特币使用的Base58字母表 */
static const char BASE58_ALPHABET[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

/* 预计算的反向映射表 */
static int8_t b58_reverse_map[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8,-1,-1,-1,-1,-1,-1,
    -1, 9,10,11,12,13,14,15,16,-1,17,18,19,20,21,-1,
    22,23,24,25,26,27,28,29,30,31,32,-1,-1,-1,-1,-1,
    -1,33,34,35,36,37,38,39,40,41,42,43,-1,44,45,46,
    47,48,49,50,51,52,53,54,55,56,57,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
};

/* 初始化反向映射表 */
__attribute__((constructor)) static void init_b58_reverse_map() {
    for (size_t i = 0; i < 58; i++) {
        uint8_t c = (uint8_t)BASE58_ALPHABET[i];
        b58_reverse_map[c] = i;
    }
}

/*
Base58编码
*/
size_t base58_encode(const uint8_t *data, size_t data_len, char *out, size_t out_size) {
    size_t zeros = 0;
    while (zeros < data_len && data[zeros] == 0)
        zeros++;
    
    // 计算所需缓冲区大小
    size_t buffer_size = (data_len - zeros) * 138 / 100 + 2;
    uint8_t *buffer = (uint8_t*)malloc(buffer_size);
    if (!buffer) return 0;
    memset(buffer, 0, buffer_size);
    
    size_t length = 0;
    
    // 添加前导零
    if (out_size < zeros) {
        free(buffer);
        return 0;
    }
    for (size_t i = 0; i < zeros; i++) {
        out[length++] = BASE58_ALPHABET[0];
    }
    
    // 使用高水位标记优化
    size_t high = buffer_size - 1;
    
    for (size_t i = zeros; i < data_len; i++) {
        int carry = data[i];
        size_t j = buffer_size - 1;
        
        // 优化进位处理
        while ((j > high) || carry) {
            if (j == 0 && carry) {
                // 处理需要扩展缓冲区的情况
                size_t new_size = buffer_size + 1;
                uint8_t *new_buffer = (uint8_t*)realloc(buffer, new_size);
                if (!new_buffer) {
                    free(buffer);
                    return 0;
                }
                buffer = new_buffer;
                // 移动现有数据
                memmove(buffer + 1, buffer, buffer_size);
                buffer[0] = 0;
                buffer_size = new_size;
                high++;
                j = 0;
            }
            
            carry += 256 * buffer[j];
            buffer[j] = carry % 58;
            carry /= 58;
            if (j == 0) break;
            j--;
        }
        
        high = j;
    }
    
    // 跳过前导零
    size_t start = 0;
    while (start < buffer_size && buffer[start] == 0)
        start++;
    
    // 计算结果长度
    size_t result_len = length + (buffer_size - start);
    if (out_size < result_len + 1) {
        free(buffer);
        return 0;
    }
    
    // 复制结果到输出缓冲区
    for (size_t j = start; j < buffer_size; j++) {
        out[length++] = BASE58_ALPHABET[buffer[j]];
    }
    out[length] = '\0';
    
    free(buffer);
    return length;
}

/*
Base58解码
*/
size_t base58_decode(const char *b58, uint8_t *out, size_t out_size) {
    // 跳过前导空格
    while (*b58 == ' ') b58++;
    
    size_t b58_len = strlen(b58);
    size_t zeros = 0;
    while (zeros < b58_len && b58[zeros] == BASE58_ALPHABET[0])
        zeros++;
    
    // 计算最大输出长度 - 使用更保守的公式
    // 原公式: (b58_len * 733) / 1000 + 1
    // 新公式: (b58_len * 138) / 100 + 1 或直接使用 b58_len
    size_t max_size = b58_len; // 更安全的做法
    
    if (max_size > out_size) {
        return 0;
    }
    
    uint8_t *bin = (uint8_t*)malloc(max_size);
    if (!bin) return 0;
    memset(bin, 0, max_size);
    
    size_t bin_size = max_size;
    size_t b58_pos = zeros;
    size_t b58_end = b58_len;
    
    // 预计算58的幂
    static const uint64_t pow58[] = {
        1,
        58,
        58 * 58,
        58 * 58 * 58,
        58 * 58 * 58 * 58,
        58ULL * 58 * 58 * 58 * 58,
        58ULL * 58 * 58 * 58 * 58 * 58
    };
    
    while (b58_pos < b58_end) {
        // 一次处理最多6个字符
        size_t group_size = (b58_end - b58_pos >= 6) ? 6 : b58_end - b58_pos;
        uint64_t value = 0;
        
        // 将字符组转换为数值
        for (size_t i = 0; i < group_size; i++) {
            int8_t digit = b58_reverse_map[(uint8_t)b58[b58_pos++]];
            if (digit == -1) {
                free(bin);
                return 0;
            }
            value = value * 58 + digit;
        }
        
        // 乘以58^group_size
        uint64_t carry = 0;
        for (size_t j = bin_size; j-- > 0; ) {
            carry += (uint64_t)bin[j] * pow58[group_size];
            bin[j] = carry & 0xFF;
            carry >>= 8;
        }
        
        if (carry) {
            free(bin);
            return 0;
        }
        
        // 加上当前值
        carry = value;
        for (size_t j = bin_size; j-- > 0; ) {
            carry += bin[j];
            bin[j] = carry & 0xFF;
            carry >>= 8;
            if (!carry) break;
        }
        
        if (carry) {
            free(bin);
            return 0;
        }
    }
    
    // 跳过前导零
    size_t i = 0;
    while (i < bin_size && bin[i] == 0)
        i++;
    
    // 复制结果到输出缓冲区 - 添加边界检查
    size_t decoded_size = zeros + (bin_size - i);
    
    // 确保不会复制超出缓冲区大小的数据
    if (decoded_size > out_size) {
        free(bin);
        return 0;
    }
    
    memset(out, 0, zeros);
    memcpy(out + zeros, bin + i, bin_size - i);
    
    free(bin);
    return decoded_size;
}

/*
Base58Check解码
*/
size_t base58_decode_check(const char *b58, uint8_t *out, size_t out_size) {
    size_t b58_len = strlen(b58);
    
    // 使用更保守的缓冲区大小
    size_t max_size = b58_len; // 更安全的做法
    
    if (max_size > out_size) {
        return 0;
    }
    
    uint8_t *bin = (uint8_t*)malloc(max_size);
    if (!bin) return 0;
    
    size_t bin_len = base58_decode(b58, bin, max_size);
    if (bin_len < 4) {
        free(bin);
        return 0;
    }

    size_t payload_len = bin_len - 4;
    if (payload_len > out_size) {
        free(bin);
        return 0;
    }

    // 计算双SHA256校验和
    uint8_t hash1[SHA256_BLOCK_SIZE], hash2[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;

    sha256_init(&ctx);
    sha256_update(&ctx, bin, payload_len);
    sha256_final(&ctx, hash1);

    sha256_init(&ctx);
    sha256_update(&ctx, hash1, SHA256_BLOCK_SIZE);
    sha256_final(&ctx, hash2);

    if (memcmp(hash2, bin + payload_len, 4) != 0) {
        free(bin);
        return 0;
    }

    memcpy(out, bin, payload_len);
    free(bin);
    return payload_len;
}

/*
Base58Check编码
*/
size_t base58_encode_check(const uint8_t *data, size_t data_len, char *out) {
    // 计算双SHA256校验和
    uint8_t hash1[SHA256_BLOCK_SIZE], hash2[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;

    sha256_init(&ctx);
    sha256_update(&ctx, data, data_len);
    sha256_final(&ctx, hash1);

    sha256_init(&ctx);
    sha256_update(&ctx, hash1, SHA256_BLOCK_SIZE);
    sha256_final(&ctx, hash2);

    // 构建完整数据 (payload + 校验和)
    uint8_t *buffer = (uint8_t*)malloc(data_len + 4);
    if (!buffer) return 0;
    
    memcpy(buffer, data, data_len);
    memcpy(buffer + data_len, hash2, 4);

    // 计算最大输出长度
    size_t max_len = BASE58_CHECK_ENCODE_OUTPUT_LEN(data_len);
    size_t result = base58_encode(buffer, data_len + 4, out, max_len);
    free(buffer);
    return result;
}

/*
封装接口
*/
int b58enc(char *b58, size_t *b58len, const uint8_t *bin, size_t binlen) {
    size_t max_len = binlen * 138 / 100 + 2;
    if (*b58len < max_len) {
        *b58len = max_len;
        return 0;
    }
    
    size_t len = base58_encode(bin, binlen, b58, *b58len);
    *b58len = len;
    return len > 0;
}

int b58tobin(uint8_t *bin, size_t *binlen, const char *b58, size_t b58len) {
    char *temp = (char*)malloc(b58len + 1);
    if (!temp) return 0;
    
    memcpy(temp, b58, b58len);
    temp[b58len] = '\0';

    size_t max_len = b58len; // 使用更保守的大小
    if (*binlen < max_len) {
        *binlen = max_len;
        free(temp);
        return 0;
    }

    size_t len = base58_decode(temp, bin, *binlen);
    free(temp);
    *binlen = len;
    return len > 0;
}
