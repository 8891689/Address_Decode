<!--  -->
<p align="center">
  <img src="https://img.shields.io/badge/Utility-Address%20to%20Hash160-blue" alt="Address to Hash160">
  <img src="https://img.shields.io/badge/Performance-Multithreaded-brightgreen" alt="Multithreaded Performance">
  <img src="https://img.shields.io/badge/Interface-CLI%20Tool-lightgrey" alt="Command Line Interface Tool">
</p>

<!--  (Supported Address Formats) -->
<p align="center">
  <img src="https://img.shields.io/badge/Format-Base58-orange" alt="Base58">
  <img src="https://img.shields.io/badge/Format-Bech32%20(SegWit)-blue" alt="Bech32 (SegWit)">
  <img src="https://img.shields.io/badge/Format-CashAddr-green" alt="CashAddr">
  <img src="https://img.shields.io/badge/Format-Legacy%20&%20P2SH-yellow" alt="Legacy & P2SH">
</p>

<!-- 🪙  (Supported Cryptocurrencies) -->
<p align="center">
  <img src="https://img.shields.io/badge/Bitcoin-BTC-orange?logo=bitcoin" alt="Bitcoin">
  <img src="https://img.shields.io/badge/Bitcoin%20Cash-BCH-green?logo=bitcoincash" alt="Bitcoin Cash">
  <img src="https://img.shields.io/badge/Litecoin-LTC-blue?logo=litecoin" alt="Litecoin">
  <img src="https://img.shields.io/badge/Bitcoin%20Gold-BTG-yellow" alt="Bitcoin Gold">
  <img src="https://img.shields.io/badge/Dash-DASH-blue?logo=dash" alt="Dash">
  <img src="https://img.shields.io/badge/Ethereum-ETH-blue?logo=ethereum" alt="Ethereum">
  <img src="https://img.shields.io/badge/Dogecoin-DOGE-yellow?logo=dogecoin" alt="Dogecoin">
</p>

<!--  -->
<p align="center">
  <img src="https://img.shields.io/badge/Language-C-blue?logo=c" alt="Language C">
  <img src="https://img.shields.io/badge/Dependencies-None-green" alt="No Dependencies">
  <img src="https://img.shields.io/badge/Platform-Linux%20&%20Windows-lightgrey?logo=linux&logoColor=black" alt="Linux & Windows">
  <a href="https://github.com/8891689/Address_Decode/releases"><img src="https://img.shields.io/github/v/release/8891689/Address_Decode" alt="Latest Release"></a>
</p>

<!--  -->
<p align="center">
  <a href="https://github.com/8891689/Address_Decode/blob/main/LICENSE"><img src="https://img.shields.io/github/license/8891689/Address_Decode" alt="License"></a>
  <a href="https://github.com/8891689/Address_Decode/commits/main"><img src="https://img.shields.io/github/last-commit/8891689/Address_Decode" alt="Last Commit"></a>
  <a href="https://github.com/8891689/Address_Decode/stargazers"><img src="https://img.shields.io/github/stars/8891689/Address_Decode?style=social" alt="GitHub Stars"></a>
</p>



# Project Overview

**Address_Decode**

Address Decode is an efficient multithreaded command-line tool designed to decode various cryptocurrency addresses (such as Bitcoin BTC, Bitcoin Cash BCH, Litecoin LTC, Bitcoin Gold BTG, dogecoin DOGE,etc.) and extract their underlying `hash160` values. `hash160` is a commonly used cryptographic hash employed in generating the core part of cryptocurrency addresses.

# Features
- **Support for Multiple Address Formats**: Supports various mainstream cryptocurrency address formats including Base58, CashAddr (Bech32), SegWit, and more.
- **Efficient Multithreading**: Utilizes POSIX threads (pthreads) to enable parallel processing, accelerating the decoding speed for large-scale address decoding.
- **Memory Optimization**: Dynamically allocates and releases memory to ensure stability when handling large amounts of data.
- **Error Handling and Statistics**: Detects errors during the decoding and extraction process, and provides statistics on the number of successful and failed decodings.
- **Flexible Command-Line Parameters**: Supports specifying input files, output files, and the number of threads to meet different requirements.


# Compilation and Usage

## Requirements
- **Operating System**: Linux (e.g., Debian/Ubuntu)
- **Compiler**: GCC (supports C99 standard)
- **Dependencies**: POSIX thread library (`pthreads`)

## Compilation

### Using Make
```bash
make
```

### Or, navigate to the project directory in the terminal and run:
```bash
gcc -static -lpthread -m64 -march=native -mtune=native -Wall -Wextra -Ofast -ftree-vectorize -funroll-all-loops -flto base58.c bech32.c cashaddr.c main.c sha256.c -o decode

```
- `-o universal_decode`: Specifies the output executable file name as `decode`.
- `-lpthread`: Links the POSIX thread library.
- `-Wall`: Enables all compiler warnings, helping to identify potential issues.

### Clean and Recompile
```bash
make clean
```
### Free access to the world's richest address rankings

http://addresses.loyce.club/

https://blockchair.com/dumps



# Running
Use the following command format to run the program:
```bash

./decode input_addresses.txt

./decode 19qZAgZM4dniNqwuYmQca7FBReTLGX9xyS

```
```
./decode
Usage  : ./decode <file or address>
  Or   : ./decode -o <Output document prefix> <file or address>
Example: 
         ./decode Input_file_containing_addresses.txt
         ./decode 19qZAgZM4dniNqwuYmQca7FBReTLGX9xyS
         ./decode -o <Output_document_prefix> <Input_file_containing_addresses.txt>
         ./decode -o <Output_document_prefix> <19qZAgZM4dniNqwuYmQca7FBReTLGX9xyS>
 Tip   : <file or address> is "-" means reading from standard input.
```

./decode Input_file_containing_addresses.txt


Directly input the name of the address file to be decoded in the directory, and it will automatically read and process the address. It will process all addresses, addresses downloaded from the website, and addresses with amounts, exclude duplicates, and sort them. By default, 4 threads execute the task. After processing, 2 files will be automatically generated, one is the successfully decoded HASH160, and the other is the failed HASH160. ETH will only remove the prefix 0x, and the BTC multi-signature script address will be removed, leaving only the 40-bit 20-byte standard HASH160 value.

```
./decode -o <Output_document_prefix> <Input_file_containing_addresses.txt>
```

-o <output_file>: Specifies the output prefix file to avoid conflicts with other outputs or overwriting other processed files.



## Example:

```
./decode Input_file_containing_addresses.txt
```

# Example and Validation

## Example Input File (`input_addresses.txt`)
```
19qZAgZM4dniNqwuYmQca7FBReTLGX9xyS	71588991775432
1PsfCHrCU3j8Y2eyPdSvfWMsk5H5pEm6j9	71588991775432
3FQGSwS6fiqLh7Uy4pAdYahYBY8TUxwwt5	71588991775432
bc1qvrha9apveexwukwvd8xa2nrknnqvqu8nd5a644	71588991775432
bc1pmy787t5td4sn8eayl97apvclzs6sju5sa73c690e2u05t8c53m6sk2n8zs	71588991775432
mpMWTjeKsfDy9xRXGLNzQ2TWHe43AyFPbA	71588991775432
n4PcVLwBH5APK98b7CRJVRaCc4sngyV7Kv	71588991775432
2N6xUWgN8HBLgtu7WjwnWAXgoPtLdEi8Vdr	71588991775432
tb1qvrha9apveexwukwvd8xa2nrknnqvqu8n8jxfwx	71588991775432
tb1pmy787t5td4sn8eayl97apvclzs6sju5sa73c690e2u05t8c53m6spz9gcl	71588991775432
qpswl5h59n8yemjee35um42vw6wvpsrs7v5urjyua2	71588991775432
qrawwuke343qt8j2yhhyzwht758kkk02tv0erlat2d	71588991775432
19qZAgZM4dniNqwuYmQca7FBReTLGX9xyS	71588991775432
1PsfCHrCU3j8Y2eyPdSvfWMsk5H5pEm6j9	71588991775432
GSgUaotJ3VQ1TKFCUi4izsb5LpFBDL8fUB	71588991775432
GgiacRB9SuLRcVxGKa736GhmfF4vmh4TpK	71588991775432
AVV8AtoHSyB7QuzXWNANGqbhWcmSFEc3Vi	71588991775432
btg1qvrha9apveexwukwvd8xa2nrknnqvqu8nmamlqa	71588991775432
LU4WRtsB9J2mdee4iuPur8JwdrpcLrUjD1	71588991775432
Li6cTWA2YhyBnqM8ZmSDwXRdxHeMuf9A56	71588991775432
MMcQkpr4cqgmVcksAh9yNDwwWEiuQW7pVB	71588991775432
ltc1qvrha9apveexwukwvd8xa2nrknnqvqu8nfg87d9	71588991775432
ltc1pmy787t5td4sn8eayl97apvclzs6sju5sa73c690e2u05t8c53m6s4wahc4	71588991775432
DDyehwVzN3gzur8WHMQB7sQnJnBdco8u1y	71588991775432
DU1kjYnqmTdR52qa8DSVDGXUdD1PBXiqb5	71588991775432
XjXPzwDF2M1JXnYVQeiqRdvyFz32GiBAAW	71588991775432
XyZW2YW6RkwigyFZFWm9X33faQrmpFXrZa	71588991775432
t1SiAB1yV2xaJyUzoVCDjhvM6gJeR39MVFR	71588991775432
t1gkGCdGLSNWj8fhsL4G3oKTnzjUAbnx5s9	71588991775432
xH9C4ZPpLMGkRE3muF3H1HXxekkZaoCnbb	71588991775432
xXBJ6AgfjmDAaQkqk75b6geeyBaKF19v2F	71588991775432
0xcacCF59299921f40D087760032a4E720aF5b68FC	71588991775432
```

## Console Output
```
Total  quantity: 32
Hash160 Success: 29 (Deduplicated and sorted)
Hash160  failed: 3

```

## Output File (`output_success.txt`)

Deduplicated and sorted
```
60efd2f42cce4cee59cc69cdd54c769cc0c070f3
9666d04e8867ce00ff5fb37ba8d413feb9ef2ef6
b860efd2f42cce4cee59cc69cdd54c769cc0c070
b8fae772d98d62059e4a25ee413aebf50f6b59ea
caccf59299921f40d087760032a4e720af5b68fc
fae772d98d62059e4a25ee413aebf50f6b59ea5b
```
----------------------------------------

Old V1.0 version, The following is the situation without deduplication and sorting.
```
60efd2f42cce4cee59cc69cdd54c769cc0c070f3
fae772d98d62059e4a25ee413aebf50f6b59ea5b
9666d04e8867ce00ff5fb37ba8d413feb9ef2ef6
60efd2f42cce4cee59cc69cdd54c769cc0c070f3
d93c7f2e8b6d6133e7a4f97dd0b31f1435097290efa38d15f9571f459f148ef5
60efd2f42cce4cee59cc69cdd54c769cc0c070f3
fae772d98d62059e4a25ee413aebf50f6b59ea5b
9666d04e8867ce00ff5fb37ba8d413feb9ef2ef6
60efd2f42cce4cee59cc69cdd54c769cc0c070f3
d93c7f2e8b6d6133e7a4f97dd0b31f1435097290efa38d15f9571f459f148ef5
60efd2f42cce4cee59cc69cdd54c769cc0c070f3
fae772d98d62059e4a25ee413aebf50f6b59ea5b
60efd2f42cce4cee59cc69cdd54c769cc0c070f3
fae772d98d62059e4a25ee413aebf50f6b59ea5b
60efd2f42cce4cee59cc69cdd54c769cc0c070f3
fae772d98d62059e4a25ee413aebf50f6b59ea5b
9666d04e8867ce00ff5fb37ba8d413feb9ef2ef6
60efd2f42cce4cee59cc69cdd54c769cc0c070f3
60efd2f42cce4cee59cc69cdd54c769cc0c070f3
fae772d98d62059e4a25ee413aebf50f6b59ea5b
9666d04e8867ce00ff5fb37ba8d413feb9ef2ef6
60efd2f42cce4cee59cc69cdd54c769cc0c070f3
d93c7f2e8b6d6133e7a4f97dd0b31f1435097290efa38d15f9571f459f148ef5
60efd2f42cce4cee59cc69cdd54c769cc0c070f3
fae772d98d62059e4a25ee413aebf50f6b59ea5b
60efd2f42cce4cee59cc69cdd54c769cc0c070f3
fae772d98d62059e4a25ee413aebf50f6b59ea5b
b860efd2f42cce4cee59cc69cdd54c769cc0c070
b8fae772d98d62059e4a25ee413aebf50f6b59ea
60efd2f42cce4cee59cc69cdd54c769cc0c070f3
fae772d98d62059e4a25ee413aebf50f6b59ea5b
caccf59299921f40d087760032a4e720af5b68fc
```

Output File (`output_failure.txt`)
```
[DECODE_FAILED] bc1pmy787t5td4sn8eayl97apvclzs6sju5sa73c690e2u05t8c53m6sk2n8zs	71588991775432
[DECODE_FAILED] tb1pmy787t5td4sn8eayl97apvclzs6sju5sa73c690e2u05t8c53m6spz9gcl	71588991775432
[DECODE_FAILED] ltc1pmy787t5td4sn8eayl97apvclzs6sju5sa73c690e2u05t8c53m6s4wahc4	71588991775432
```
These are multi-signature script addresses. In fact, they are not the hash values ​​of the public key, but the hash values ​​of the script. They are not 40 characters of letters and numbers at all, that's more than 20 bytes. They cannot be used by other programs because other programs can only calculate the hash value from the public key and then encode it into various addresses.

## Run Command
```
./decode 19qZAgZM4dniNqwuYmQca7FBReTLGX9xyS
```
## Output
```
60efd2f42cce4cee59cc69cdd54c769cc0c070f3
```


# Notes
- For BCH addresses, the `00` prefix of `hash160` is correctly removed, and only the valid `hash160` is extracted.
- For Segwit and hex-encoded addresses, the `hash160` is correctly extracted.
- For ETH addresses, only the `0x` prefix is ​​removed, and the subsequent value is preserved for use by other programs.

# Dependencies

None. You can compile as long as you download the compiler.

# License
This project uses the MIT license. See the [LICENSE](LICENSE) file for details.

## Acknowledgements

Special thanks to Gemini and DeepSeek for their assistance with testing and translation.

# Sponsorship

If this project is helpful to you, please consider sponsoring. This is the greatest support I can give, and I am deeply grateful. Thank you.

- **BTC**: bc1qt3nh2e6gjsfkfacnkglt5uqghzvlrr6jahyj2k
- **ETH**: 0xD6503e5994bF46052338a9286Bc43bC1c3811Fa1
- **DOGE**: DTszb9cPALbG9ESNJMFJt4ECqWGRCgucky
- **TRX**: TAHUmjyzg7B3Nndv264zWYUhQ9HUmX4Xu4

# Contact
If you have any questions, suggestions, or feedback, please feel free to contact us.

# Disclaimer
This tool is intended for educational and research purposes only. Users are responsible for any risks and liabilities arising from the use of this tool. The developers are not liable for any losses resulting from the use of this tool.
