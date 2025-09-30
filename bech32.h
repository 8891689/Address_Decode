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
#ifndef BECH32_H
#define BECH32_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * segwit_addr_encode - 使用 Bech32 格式对 segwit 地址进行编码
 *
 * @output: 输出缓冲区，用于存放 null 结尾的地址字符串（调用者保证足够大）
 * @witver: witness 版本（0～16）
 * @witprog: witness 程序（二进制数据）
 * @witprog_len: witness 程序的字节长度
 *
 * 成功返回 1，失败返回 0。
 */
int segwit_addr_encode(char *output, const char *hrp, int witver, const uint8_t *witprog, size_t witprog_len);

/**
 * segwit_addr_decode - 解码 Bech32 格式的 segwit 地址
 *
 * @addr: 输入的 Bech32 地址字符串
 * @witver: 输出参数，保存解析出的 witness 版本
 * @witprog: 输出缓冲区，用于保存解析出的 witness 程序（二进制数据）
 * @witprog_len: 输入时为 witprog 缓冲区大小；输出时保存实际数据长度
 *
 * 成功返回 1，失败返回 0。
 */
int segwit_addr_decode(const char *addr, const char *hrp, int *witver, uint8_t *witprog, size_t *witprog_len);

#ifdef __cplusplus
}
#endif

#endif /* bech32_h */

