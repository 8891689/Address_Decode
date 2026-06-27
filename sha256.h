/* sha256.h */
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

#ifndef SHA256_H
#define SHA256_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHA256_BLOCK_SIZE 32

typedef struct { 
 uint8_t data[64]; 
 uint32_t datalen; 
 uint64_t bitlen;
 uint32_t state[8]; 
} SHA256_CTX;


void sha256_init(SHA256_CTX *ctx); 
void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len); 
void sha256_final(SHA256_CTX *ctx, uint8_t hash[]); void sha256(const uint8_t *data, size_t len, uint8_t *hash);



#ifdef __cplusplus
} // extern "C"
#endif

#endif // SHA256_H
