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
   Author: 8891689 https://www.8891689.com/ 
*/
#ifndef BASE58_H
#define BASE58_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 预计算输出缓冲区大小 */
#define BASE58_ENCODE_OUTPUT_LEN(len) (((len) * 138 / 100) + 2)
#define BASE58_DECODE_OUTPUT_LEN(len) (((len) * 733 / 1000) + 1)
#define BASE58_CHECK_ENCODE_OUTPUT_LEN(len) BASE58_ENCODE_OUTPUT_LEN((len) + 4)
#define BASE58_CHECK_DECODE_OUTPUT_LEN(len) BASE58_DECODE_OUTPUT_LEN(len)

/* 核心编码函数 */
size_t base58_encode(const uint8_t *data, size_t data_len, char *out, size_t out_size);
size_t base58_decode(const char *b58, uint8_t *out, size_t out_size);
size_t base58_encode_check(const uint8_t *data, size_t data_len, char *out);
size_t base58_decode_check(const char *b58, uint8_t *out, size_t out_size);

/* 封装接口 */
int b58enc(char *b58, size_t *b58len, const uint8_t *bin, size_t binlen);
int b58tobin(uint8_t *bin, size_t *binlen, const char *b58, size_t b58len);

#ifdef __cplusplus
}
#endif

#endif // BASE58_H
