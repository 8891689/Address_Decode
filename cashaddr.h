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
#ifndef CASHADDR_H
#define CASHADDR_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 解析后的现金地址结果 */
typedef struct {
    char prefix[32];     /* 地址前缀 */
    int version;         /* 版本 */
    char type[16];       /* 地址类型，例如 "P2PKH" 或 "P2SH" 或 "未知类型" */
    char hash160[41];    /* 40字符十六进制字符串，附带结束符 */
} CashAddrResult;

/* 解码现金地址 */
int decode_cashaddr(const char *address, CashAddrResult *result);

/* 编码现金地址 */
int encode_cashaddr(const char *prefix, int version, const char *hash160,
                    char *out_address, size_t out_size);

#ifdef __cplusplus
}
#endif

#endif /* CASHADDR_H */
