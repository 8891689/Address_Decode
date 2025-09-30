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
#include "cashaddr.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* CashAddr中使用的Base32字符集 */
static const char CHARSET[] = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

/* 完整初始化的Base32解碼表 */
static const uint8_t BASE32_DECODE[256] = {
    // 使用指定初始化
    ['q'] = 0, ['Q'] = 0,
    ['p'] = 1, ['P'] = 1,
    ['z'] = 2, ['Z'] = 2,
    ['r'] = 3, ['R'] = 3,
    ['y'] = 4, ['Y'] = 4,
    ['9'] = 5,
    ['x'] = 6, ['X'] = 6,
    ['8'] = 7,
    ['g'] = 8, ['G'] = 8,
    ['f'] = 9, ['F'] = 9,
    ['2'] = 10,
    ['t'] = 11, ['T'] = 11,
    ['v'] = 12, ['V'] = 12,
    ['d'] = 13, ['D'] = 13,
    ['w'] = 14, ['W'] = 14,
    ['0'] = 15,
    ['s'] = 16, ['S'] = 16,
    ['3'] = 17,
    ['j'] = 18, ['J'] = 18,
    ['n'] = 19, ['N'] = 19,
    ['5'] = 20,
    ['4'] = 21,
    ['k'] = 22, ['K'] = 22,
    ['h'] = 23, ['H'] = 23,
    ['c'] = 24, ['C'] = 24,
    ['e'] = 25, ['E'] = 25,
    ['6'] = 26,
    ['m'] = 27, ['M'] = 27,
    ['u'] = 28, ['U'] = 28,
    ['a'] = 29, ['A'] = 29,
    ['7'] = 30,
    ['l'] = 31, ['L'] = 31
};

/* 優化後的多項式模運算 */
static uint64_t polymod_scalar(const int *values, size_t count) {
    uint64_t c = 1;
    size_t i = 0;
    // 每次處理多個值，例如4個一組
    for (; i + 4 <= count; i += 4) {
        uint8_t d0 = values[i];
        uint8_t d1 = values[i+1];
        uint8_t d2 = values[i+2];
        uint8_t d3 = values[i+3];
        
        // 處理d0
        uint8_t c0 = c >> 35;
        c = ((c & 0x07ffffffffULL) << 5) | d0;
        c ^= ((c0 & 0x01) ? 0x98f2bc8e61ULL : 0);
        c ^= ((c0 & 0x02) ? 0x79b76d99e2ULL : 0);
        c ^= ((c0 & 0x04) ? 0xf33e5fb3c4ULL : 0);
        c ^= ((c0 & 0x08) ? 0xae2eabe2a8ULL : 0);
        c ^= ((c0 & 0x10) ? 0x1e4f43e470ULL : 0);
        
        // 處理d1
        c0 = c >> 35;
        c = ((c & 0x07ffffffffULL) << 5) | d1;
        c ^= ((c0 & 0x01) ? 0x98f2bc8e61ULL : 0);
        c ^= ((c0 & 0x02) ? 0x79b76d99e2ULL : 0);
        c ^= ((c0 & 0x04) ? 0xf33e5fb3c4ULL : 0);
        c ^= ((c0 & 0x08) ? 0xae2eabe2a8ULL : 0);
        c ^= ((c0 & 0x10) ? 0x1e4f43e470ULL : 0);
        
        // 處理d2
        c0 = c >> 35;
        c = ((c & 0x07ffffffffULL) << 5) | d2;
        c ^= ((c0 & 0x01) ? 0x98f2bc8e61ULL : 0);
        c ^= ((c0 & 0x02) ? 0x79b76d99e2ULL : 0);
        c ^= ((c0 & 0x04) ? 0xf33e5fb3c4ULL : 0);
        c ^= ((c0 & 0x08) ? 0xae2eabe2a8ULL : 0);
        c ^= ((c0 & 0x10) ? 0x1e4f43e470ULL : 0);
        
        // 處理d3
        c0 = c >> 35;
        c = ((c & 0x07ffffffffULL) << 5) | d3;
        c ^= ((c0 & 0x01) ? 0x98f2bc8e61ULL : 0);
        c ^= ((c0 & 0x02) ? 0x79b76d99e2ULL : 0);
        c ^= ((c0 & 0x04) ? 0xf33e5fb3c4ULL : 0);
        c ^= ((c0 & 0x08) ? 0xae2eabe2a8ULL : 0);
        c ^= ((c0 & 0x10) ? 0x1e4f43e470ULL : 0);
    }
    
    // 處理剩餘元素
    for (; i < count; i++) {
        uint8_t d = values[i];
        uint8_t c0 = c >> 35;
        c = ((c & 0x07ffffffffULL) << 5) | d;
        c ^= ((c0 & 0x01) ? 0x98f2bc8e61ULL : 0);
        c ^= ((c0 & 0x02) ? 0x79b76d99e2ULL : 0);
        c ^= ((c0 & 0x04) ? 0xf33e5fb3c4ULL : 0);
        c ^= ((c0 & 0x08) ? 0xae2eabe2a8ULL : 0);
        c ^= ((c0 & 0x10) ? 0x1e4f43e470ULL : 0);
    }
    
    return c ^ 1;
}

/* 自動選擇最佳的多項式模實現 */
static uint64_t _polymod(const int *values, size_t count) {
    return polymod_scalar(values, count);
}

/* 計算CashAddr的校驗和 */
static void _calculate_checksum(const char *prefix, const int *payload, int payload_len, int checksum[8]) {
    int prefix_len = (int)strlen(prefix);
    int data_len = prefix_len + 1 + payload_len + 8;
    int data[128]; // 棧上分配，避免malloc
    
    // 轉換前綴：每個字符取低5位
    for (int i = 0; i < prefix_len; i++) {
        data[i] = tolower(prefix[i]) & 0x1f;
    }
    data[prefix_len] = 0;
    
    // 拷貝payload
    memcpy(data + prefix_len + 1, payload, payload_len * sizeof(int));
    
    // 後續8個0
    memset(data + prefix_len + 1 + payload_len, 0, 8 * sizeof(int));
    
    uint64_t poly = _polymod(data, data_len);
    for (int i = 0; i < 8; i++) {
        checksum[i] = (poly >> (5 * (7 - i))) & 0x1f;
    }
}

/* 將字節數組轉換為5位數組，返回5位數組長度 */
static int _pack_5bit(const unsigned char *data, int data_len, int *out, int max_out) {
    int buffer = 0, bits = 0;
    int count = 0;
    
    for (int i = 0; i < data_len; i++) {
        buffer = (buffer << 8) | data[i];
        bits += 8;
        while (bits >= 5) {
            bits -= 5;
            if (count >= max_out) return -1;
            out[count++] = (buffer >> bits) & 0x1f;
            buffer &= (1 << bits) - 1;
        }
    }
    
    if (bits > 0) {
        if (count >= max_out) return -1;
        out[count++] = (buffer << (5 - bits)) & 0x1f;
    }
    
    return count;
}

/* 將5位數組還原為字節數組，返回還原後的字節數 */
static int _unpack_5bit(const int *data, int data_len, unsigned char *out, int max_out) {
    int buffer = 0, bits = 0;
    int count = 0;
    
    for (int i = 0; i < data_len; i++) {
        buffer = (buffer << 5) | data[i];
        bits += 5;
        if (bits >= 8) {
            bits -= 8;
            if (count >= max_out) return -1;
            out[count++] = (buffer >> bits) & 0xff;
            buffer &= (1 << bits) - 1;
        }
    }
    
    // 處理剩餘位（如果有）
    if (bits > 0) {
        // 不需要特別處理
    }
    
    return count;
}

/* 十六進制字符到數字的查找表 */
static const int HEX_LOOKUP[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,
    -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

/* 將十六進制字符串轉換為字節數組，返回字節數 */
static int hexstr2bytes(const char *hex, unsigned char *out, int max_out) {
    int len = (int)strlen(hex);
    if (len % 2 != 0) return -1;
    int bytes = len / 2;
    if (bytes > max_out) return -1;
    
    for (int i = 0; i < bytes; i++) {
        int hi = HEX_LOOKUP[(unsigned char)hex[2*i]];
        int lo = HEX_LOOKUP[(unsigned char)hex[2*i+1]];
        if (hi < 0 || lo < 0) return -1;
        out[i] = (unsigned char)((hi << 4) | lo);
    }
    return bytes;
}

/* 解碼現金地址，結果寫入result結構中，返回0表示成功 */
int decode_cashaddr(const char *address, CashAddrResult *result) {
    char prefix[32] = {0};
    const char *base32_part = NULL;
    const char *colon = strchr(address, ':');
    
    if (colon) {
        size_t pre_len = colon - address;
        if (pre_len >= sizeof(prefix)) return -1;
        memcpy(prefix, address, pre_len);
        prefix[pre_len] = '\0';
        base32_part = colon + 1;
    } else {
        strcpy(prefix, "bitcoincash");
        base32_part = address;
    }

    int base32_len = (int)strlen(base32_part);
    if (base32_len < 8) {
        return -1;
    }
    
    int packed[128] = {0};
    for (int i = 0; i < base32_len; i++) {
        uint8_t val = BASE32_DECODE[(unsigned char)base32_part[i]];
        if (val == 0xFF) {
            return -1; // 無效字符
        }
        packed[i] = val;
    }

    int payload_len = base32_len - 8;
    
    // 構造用於校驗和計算的數據數組
    int prefix_len = (int)strlen(prefix);
    int data_for_check_len = prefix_len + 1 + base32_len;
    int data_for_check[150] = {0}; // 確保足夠大
    
    // 轉換前綴：每個字符取低5位
    for (int i = 0; i < prefix_len; i++) {
        data_for_check[i] = tolower(prefix[i]) & 0x1f;
    }
    data_for_check[prefix_len] = 0;
    memcpy(data_for_check + prefix_len + 1, packed, base32_len * sizeof(int));
    
    uint64_t poly = _polymod(data_for_check, data_for_check_len);
    if (poly != 0) {
        return -1; // 校驗和錯誤
    }

    // 解包有效載荷（轉換為字節數組）
    unsigned char payload_bytes[65] = {0}; // payload最長為(104-8)*5/8 = 60字節，留些餘裕
    int payload_bytes_len = _unpack_5bit(packed, payload_len, payload_bytes, sizeof(payload_bytes));
    if (payload_bytes_len < 1) {
        return -1;
    }

    // 解析版本字節
    int version_byte = payload_bytes[0];
    int size_bits = (version_byte >> 0) & 0x07;
    int type_bits = (version_byte >> 3) & 0x0f; // 應該是 & 0x0f, 不是 & 0x1f
    
    // 根據size_bits計算期望的hash長度
    int expected_hash_len = 0;
    switch(size_bits) {
        case 0: expected_hash_len = 20; break; // 160 bits
        case 1: expected_hash_len = 24; break; // 192 bits
        case 2: expected_hash_len = 28; break; // 224 bits
        case 3: expected_hash_len = 32; break; // 256 bits
        case 4: expected_hash_len = 40; break; // 320 bits
        case 5: expected_hash_len = 48; break; // 384 bits
        case 6: expected_hash_len = 56; break; // 448 bits
        case 7: expected_hash_len = 64; break; // 512 bits
    }
    
    if (payload_bytes_len != 1 + expected_hash_len) {
        return -1; // payload長度與版本字節不匹配
    }

    const char *addr_type;
    if (type_bits == 0) {
        addr_type = "P2PKH";
    } else if (type_bits == 1) {
        addr_type = "P2SH";
    } else {
        addr_type = "未知類型";
    }

    // 提取hash
    char hash_hex[129] = {0};
    for (int i = 0; i < expected_hash_len; i++) {
        sprintf(hash_hex + i * 2, "%02x", payload_bytes[i+1]);
    }

    /* 填充返回結果 */
    strncpy(result->prefix, prefix, sizeof(result->prefix)-1);
    result->prefix[sizeof(result->prefix)-1] = '\0';
    
    result->version = type_bits; // The "version" in common parlance is the type.
    
    strncpy(result->type, addr_type, sizeof(result->type)-1);
    result->type[sizeof(result->type)-1] = '\0';
    
    strncpy(result->hash160, hash_hex, sizeof(result->hash160)-1);
    result->hash160[sizeof(result->hash160)-1] = '\0';
    
    return 0;
}

/* 編碼現金地址 */
int encode_cashaddr(const char *prefix, int version, const char *hash160,
                    char *out_address, size_t out_size) {
    // 使用 version 参数作为类型位
    int type_bits = version;
    
    size_t hash_len = strlen(hash160);
    if (hash_len % 2 != 0) return -1;
    size_t hash_bytes_len = hash_len / 2;

    int size_bits = -1;
    switch(hash_bytes_len) {
        case 20: size_bits = 0; break;
        case 24: size_bits = 1; break;
        case 28: size_bits = 2; break;
        case 32: size_bits = 3; break;
        case 40: size_bits = 4; break;
        case 48: size_bits = 5; break;
        case 56: size_bits = 6; break;
        case 64: size_bits = 7; break;
        default: return -1; // 無效的hash長度
    }

    unsigned char hash_bytes[64];
    if (hexstr2bytes(hash160, hash_bytes, sizeof(hash_bytes)) != (int)hash_bytes_len) {
        return -1; // 十六進制轉換失敗
    }
    
    unsigned char payload_bytes[65];
    payload_bytes[0] = (unsigned char)((type_bits << 3) | (size_bits & 0x07));
    memcpy(payload_bytes + 1, hash_bytes, hash_bytes_len);

    int payload_packed[120] = {0};
    int payload_packed_len = _pack_5bit(payload_bytes, 1 + hash_bytes_len, payload_packed, 120);
    if (payload_packed_len < 0) return -1;

    int checksum[8] = {0};
    _calculate_checksum(prefix, payload_packed, payload_packed_len, checksum);

    int full_len = payload_packed_len + 8;
    char base32_part[130] = {0};
    for (int i = 0; i < payload_packed_len; i++) {
        base32_part[i] = CHARSET[payload_packed[i]];
    }
    for (int i = 0; i < 8; i++) {
        base32_part[payload_packed_len + i] = CHARSET[checksum[i]];
    }
    base32_part[full_len] = '\0';

    /* 拼接前綴和Base32部分 */
    int n = snprintf(out_address, out_size, "%s:%s", prefix, base32_part);
    if (n < 0 || (size_t)n >= out_size) return -1;
    
    return 0;
}
