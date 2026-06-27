/*
MIT License

Copyright (c) 2025 8891689

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. *https://www.8891689.com/ 
 */

// Description: A multi-threaded tool to decode various cryptocurrency addresses from a large file.
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>


// ======================= CROSS-PLATFORM COMPATIBILITY: TIME FUNCTIONS =======================
#ifdef _WIN32
#include <windows.h>
#define FSEEK _fseeki64
#define FTELL _ftelli64
#define OFF_T __int64

// Windows-specific high-precision timer implementation
typedef struct {
    LARGE_INTEGER start;
    LARGE_INTEGER frequency;
} TimerData;

static void start_timer(TimerData *timer) {
    QueryPerformanceFrequency(&timer->frequency);
    QueryPerformanceCounter(&timer->start);
}

static double get_elapsed_seconds(TimerData *timer) {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (double)(now.QuadPart - timer->start.QuadPart) / timer->frequency.QuadPart;
}

// Provide getline for Windows as it's a GNU extension
typedef long long ssize_t;
ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;
    if (lineptr == NULL || stream == NULL || n == NULL) { return -1; }
    bufptr = *lineptr;
    size = *n;
    c = fgetc(stream);
    if (c == EOF) { return -1; }
    if (bufptr == NULL) {
        bufptr = malloc(128);
        if (bufptr == NULL) { return -1; }
        size = 128;
    }
    p = bufptr;
    while(c != EOF) {
        if ((size_t)(p - bufptr) >= (size - 1)) {
            size_t offset = p - bufptr;
            size += 128;
            char *new_buf = realloc(bufptr, size);
            if (new_buf == NULL) { return -1; }
            bufptr = new_buf;
            p = bufptr + offset;
        }
        *p++ = c;
        if (c == '\n') { break; }
        c = fgetc(stream);
    }
    *p++ = '\0';
    *lineptr = bufptr;
    *n = size;
    return p - bufptr - 1;
}

#define usleep(x) Sleep((x)/1000)

#else // For non-Windows (Linux, macOS, etc.)
#include <unistd.h>
#define FSEEK fseeko
#define FTELL ftello
#define OFF_T off_t

// POSIX-standard timer implementation
typedef struct {
    struct timespec start;
} TimerData;

static void start_timer(TimerData *timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->start);
}

static double get_elapsed_seconds(TimerData *timer) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (double)(now.tv_sec - timer->start.tv_sec) + (double)(now.tv_nsec - timer->start.tv_nsec) / 1e9;
}

#endif
// =================================== END OF COMPATIBILITY FIXES ===================================


#include "sha256.h"
#include "base58.h"
#include "bech32.h"
#include "cashaddr.h"

/* =========================================================================
 * 1. GLOBAL VARIABLES & SHARED RESOURCES
 * ========================================================================= */

#define BUFFER_SIZE 10000
typedef struct {
    char *line;
    size_t length;
} LineData;

LineData line_buffer[BUFFER_SIZE];
size_t count_in_buffer = 0;
size_t read_idx = 0;
size_t write_idx = 0;
bool producer_finished = false;

pthread_mutex_t buffer_mutex;
pthread_cond_t buffer_not_full;
pthread_cond_t buffer_not_empty;

pthread_mutex_t output_mutex;
FILE *fout_failure = NULL;
volatile size_t failure_count = 0;

pthread_mutex_t results_mutex;
char **success_results = NULL;
volatile size_t success_capacity = 0;
volatile size_t success_count_atomic = 0;

volatile OFF_T total_bytes = 0;
volatile OFF_T processed_bytes = 0;
volatile bool display_thread_done = false;

typedef struct {
    TimerData *timer;
} ProgressData;

/* =========================================================================
 * 2. HELPER FUNCTIONS
 * ========================================================================= */

static void bytes_to_hex(const unsigned char *bytes, size_t len, char *hex_str) {
    for (size_t i = 0; i < len; i++) {
        sprintf(&hex_str[i * 2], "%02x", bytes[i]);
    }
    hex_str[len * 2] = '\0';
}

static int hex_to_bytes(const char *hex_str, unsigned char *bytes_out, size_t max_len) {
    size_t len = strlen(hex_str);
    if (len % 2 != 0) return 0;
    size_t bytes_len = len / 2;
    if (bytes_len > max_len) return 0;
    for (size_t i = 0; i < bytes_len; i++) {
        unsigned int val;
        if (sscanf(hex_str + (i * 2), "%2x", &val) != 1) return 0;
        bytes_out[i] = (unsigned char)val;
    }
    return (int)bytes_len;
}

int compare_strings(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

/* =========================================================================
 * 3. CORE DECODING LOGIC
 * ========================================================================= */

static int decode_address_general(const char *addr_str, unsigned char *out_bytes, size_t out_capacity, size_t *out_len) {
    unsigned char temp_decoded_buf[256];
    size_t current_len = 0;
    int witver;

    current_len = base58_decode_check(addr_str, temp_decoded_buf, sizeof(temp_decoded_buf));
    if (current_len == 21) { // P2PKH
        if (out_capacity < 20) return 0;
        memcpy(out_bytes, temp_decoded_buf + 1, 20);
        *out_len = 20;
        return 1;
    } else if (current_len == 22) { // P2SH
        if (out_capacity < 20) return 0;
        memcpy(out_bytes, temp_decoded_buf + 2, 20);
        *out_len = 20;
        return 1;
    }

    const char *hrps[] = {"bc", "tb", "ltc", "tltc", "btg", NULL};
    for (int i = 0; hrps[i] != NULL; ++i) {
        size_t segwit_prog_len_val = sizeof(temp_decoded_buf);
        if (segwit_addr_decode(addr_str, hrps[i], &witver, temp_decoded_buf, &segwit_prog_len_val)) {
            if (out_capacity < segwit_prog_len_val) return 0;
            memcpy(out_bytes, temp_decoded_buf, segwit_prog_len_val);
            *out_len = segwit_prog_len_val;
            return 1;
        }
    }

    CashAddrResult cash_result;
    if (decode_cashaddr(addr_str, &cash_result) == 0) {
        current_len = hex_to_bytes(cash_result.hash160, out_bytes, out_capacity);
        if (current_len == 20) {
            *out_len = 20;
            return 1;
        }
    }

    if (strncmp(addr_str, "0x", 2) == 0 || strncmp(addr_str, "0X", 2) == 0) {
        current_len = hex_to_bytes(addr_str + 2, out_bytes, out_capacity);
    } else {
        current_len = hex_to_bytes(addr_str, out_bytes, out_capacity);
    }

    if (current_len > 0) {
        *out_len = current_len;
        return 1;
    }

    return 0;
}

/* =========================================================================
 * 4. CONCURRENT WORKERS (THREADS)
 * ========================================================================= */

void* display_progress(void *arg) {
    ProgressData *p_data = (ProgressData*)arg;
    TimerData *timer = p_data->timer;
    const int BAR_WIDTH = 40;

    while (!display_thread_done) {
        if (total_bytes <= 0) {
            usleep(100000);
            continue;
        }
        double percentage = (double)processed_bytes * 100.0 / total_bytes;
        int pos = (int)((double)BAR_WIDTH * processed_bytes / total_bytes);

        double elapsed_sec = get_elapsed_seconds(timer);
        double speed = elapsed_sec > 0.01 ? (double)processed_bytes / (1024.0 * 1024.0) / elapsed_sec : 0.0;

        fprintf(stderr, "\rDecoding: [");
        for (int i = 0; i < BAR_WIDTH; ++i) {
            if (i < pos) fprintf(stderr, "=");
            else if (i == pos) fprintf(stderr, ">");
            else fprintf(stderr, " ");
        }
        fprintf(stderr, "] %6.2f%% | %.2f MB/s ", percentage > 100.0 ? 100.0 : percentage, speed);
        fflush(stderr);
        usleep(100000);
    }

    double total_elapsed_sec = get_elapsed_seconds(timer);
    fprintf(stderr, "\rDecoding: [========================================] 100.00%% | Total Time: %.2fs\n", total_elapsed_sec);
    fflush(stderr);
    return NULL;
}

void* thread_process(void *arg) {
    (void)arg;
    while (1) {
        pthread_mutex_lock(&buffer_mutex);
        while (count_in_buffer == 0 && !producer_finished) {
            pthread_cond_wait(&buffer_not_empty, &buffer_mutex);
        }
        if (count_in_buffer == 0 && producer_finished) {
            pthread_mutex_unlock(&buffer_mutex);
            break;
        }

        LineData data = line_buffer[read_idx];
        read_idx = (read_idx + 1) % BUFFER_SIZE;
        count_in_buffer--;
        pthread_cond_signal(&buffer_not_full);
        pthread_mutex_unlock(&buffer_mutex);

        char address_part_buffer[1024];
        address_part_buffer[0] = '\0';
        bool truncated = false;
        
        char *p = data.line;
        while (*p && isspace((unsigned char)*p)) p++;
        
        char *end = p;
        while (*end && !isspace((unsigned char)*end)) end++;

        size_t addr_len = end - p;
        if (addr_len > 0) {
            size_t copy_len = addr_len;
            if (copy_len >= sizeof(address_part_buffer)) {
                copy_len = sizeof(address_part_buffer) - 1;
                truncated = true;
            }
            strncpy(address_part_buffer, p, copy_len);
            address_part_buffer[copy_len] = '\0';
        }

        if (address_part_buffer[0] == '\0') {
            pthread_mutex_lock(&output_mutex);
            fprintf(fout_failure, "[EMPTY_ADDRESS] %s", data.line);
            __atomic_add_fetch(&failure_count, 1, __ATOMIC_RELAXED);
            pthread_mutex_unlock(&output_mutex);
        } else if (truncated) {
            pthread_mutex_lock(&output_mutex);
            fprintf(fout_failure, "[TRUNCATED_ADDR] %s", data.line);
            __atomic_add_fetch(&failure_count, 1, __ATOMIC_RELAXED);
            pthread_mutex_unlock(&output_mutex);
        } else {
            unsigned char extracted_bytes[128];
            size_t extracted_len = 0;
            if (decode_address_general(address_part_buffer, extracted_bytes, sizeof(extracted_bytes), &extracted_len)) {
                if (extracted_len == 20) {
                    char hex_str[41];
                    bytes_to_hex(extracted_bytes, 20, hex_str);
                    char *result_str = strdup(hex_str);
                    if (result_str) {
                        pthread_mutex_lock(&results_mutex);

                        if (success_count_atomic >= success_capacity) {
                            size_t new_capacity = success_capacity == 0 ? 1000000 : success_capacity * 2;
                            char **new_results = realloc(success_results, new_capacity * sizeof(char*));
                            if (new_results) {
                                success_results = new_results;
                                success_capacity = new_capacity;
                            } else {
                                free(result_str);
                                pthread_mutex_unlock(&results_mutex);
                                continue;
                            }
                        }
                        
                        success_results[success_count_atomic] = result_str;
                        success_count_atomic++;

                        pthread_mutex_unlock(&results_mutex);
                    }
                } else {
                    char hex_str[257];
                    bytes_to_hex(extracted_bytes, extracted_len, hex_str);
                    pthread_mutex_lock(&output_mutex);
                    fprintf(fout_failure, "[NON_STANDARD_HASH: %s] %s", hex_str, data.line);
                    __atomic_add_fetch(&failure_count, 1, __ATOMIC_RELAXED);
                    pthread_mutex_unlock(&output_mutex);
                }
            } else {
                pthread_mutex_lock(&output_mutex);
                fprintf(fout_failure, "[DECODE_FAILED: %s] %s", address_part_buffer, data.line);
                __atomic_add_fetch(&failure_count, 1, __ATOMIC_RELAXED);
                pthread_mutex_unlock(&output_mutex);
            }
        }
        free(data.line);
        __atomic_add_fetch(&processed_bytes, data.length, __ATOMIC_RELAXED);
    }
    return NULL;
}

/* =========================================================================
 * 5. MAIN FUNCTION (Entry Point)
 * ========================================================================= */

int main(int argc, char *argv[]) {
    char *input_source = NULL;
    char *output_filename = "output.txt"; // Default filename
    int thread_count = 4; // Default threads

    // 參數解析循環
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_filename = argv[++i];
            } else {
                fprintf(stderr, "Error: -o argument missing\n");
                return 1;
            }
        } 
        else if (strcmp(argv[i], "-t") == 0) {
            if (i + 1 < argc) {
                thread_count = atoi(argv[++i]);
                if (thread_count < 1) thread_count = 1;
            } else {
                fprintf(stderr, "Error: -t argument missing\n");
                return 1;
            }
        }
        else {
            input_source = argv[i];
        }
    }

    if (input_source == NULL) {
        fprintf(stderr, "Usage  : %s [options] <file or address>\n\n", argv[0]);
        
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -o <filename>  Specify the output filename for successfully decoded results.\n");
        fprintf(stderr, "                 Failures will be saved to '<filename>_failure.txt'.\n");
        fprintf(stderr, "                 (Default: output.txt)\n\n");
        
        fprintf(stderr, "  -t <number>    Set the number of threads for parallel processing.\n");
        fprintf(stderr, "                 (Default: 4)\n\n");
        
        fprintf(stderr, "Examples:\n");
        fprintf(stderr, "  %s input_addresses.txt\n", argv[0]);
        fprintf(stderr, "  %s -t 8 input_addresses.txt\n", argv[0]);
        fprintf(stderr, "  %s -o my_result.txt -t 16 input_addresses.txt\n", argv[0]);
        fprintf(stderr, "  %s 19qZAgZM4dniNqwuYmQca7FBReTLGX9xyS\n\n", argv[0]);
        
        fprintf(stderr, "Tips:\n");
        fprintf(stderr, "  <file or address> can be \"-\" to read from standard input.\n");
        fprintf(stderr, "  Technical Support: www.8891689.com\n");
        return 1;
    }

    FILE *fin = NULL;
    if (strcmp(input_source, "-") == 0) {
        fin = stdin;
    } else {
        fin = fopen(input_source, "r");
    }

    if (!fin) {
        // Single address mode
        unsigned char bytes[128];
        size_t len = 0;
        if (decode_address_general(input_source, bytes, sizeof(bytes), &len)) {
            if (len == 20) {
                char hex_str[41];
                bytes_to_hex(bytes, 20, hex_str);
                fprintf(stdout, "%s\n", hex_str);
            } else {
                char hex_str[257];
                bytes_to_hex(bytes, len, hex_str);
                fprintf(stderr, "[Error] Decoded, but not a standard 20-byte hash: %s\n", hex_str);
            }
        } else {
            fprintf(stderr, "[Error] Failed to decode address.\n");
        }
        return 0;
    }

    if (fin != stdin) {
        FSEEK(fin, 0, SEEK_END);
        total_bytes = FTELL(fin);
        FSEEK(fin, 0, SEEK_SET);
    }

    // Build failure filename: output_filename + "_failure.txt"
    char outFileFailurePath[1024];
    snprintf(outFileFailurePath, sizeof(outFileFailurePath), "%s_failure.txt", output_filename);
    
    fout_failure = fopen(outFileFailurePath, "w");
    if (!fout_failure) {
        perror("Failed to open failure output file");
        fclose(fin);
        return 1;
    }

    pthread_mutex_init(&buffer_mutex, NULL);
    pthread_mutex_init(&output_mutex, NULL);
    pthread_mutex_init(&results_mutex, NULL);
    pthread_cond_init(&buffer_not_full, NULL);
    pthread_cond_init(&buffer_not_empty, NULL);

    pthread_t *threads = malloc(thread_count * sizeof(pthread_t));
    for (int i = 0; i < thread_count; i++) {
        pthread_create(&threads[i], NULL, thread_process, NULL);
    }

    TimerData timer;
    start_timer(&timer);
    pthread_t progress_thread_id;
    ProgressData p_data = { .timer = &timer };
    if (fin != stdin) {
        pthread_create(&progress_thread_id, NULL, display_progress, &p_data);
    }

    // Producer loop
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, fin)) != -1) {
        pthread_mutex_lock(&buffer_mutex);
        while (count_in_buffer == BUFFER_SIZE) {
            pthread_cond_wait(&buffer_not_full, &buffer_mutex);
        }
        
        line_buffer[write_idx].line = line;
        line_buffer[write_idx].length = read;
        
        line = NULL; 
        len = 0;
        
        write_idx = (write_idx + 1) % BUFFER_SIZE;
        count_in_buffer++;
        pthread_cond_signal(&buffer_not_empty);
        pthread_mutex_unlock(&buffer_mutex);
    }
    
    if (line) {
        free(line);
    }

    pthread_mutex_lock(&buffer_mutex);
    producer_finished = true;
    pthread_cond_broadcast(&buffer_not_empty);
    pthread_mutex_unlock(&buffer_mutex);

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    if (fin != stdin) {
        display_thread_done = true;
        pthread_join(progress_thread_id, NULL);
    }

    fprintf(stderr, "Processing complete. Now sorting and filtering unique results...\n");
    size_t final_success_count = success_count_atomic;
    
    if (final_success_count > 0) {
        qsort(success_results, final_success_count, sizeof(char*), compare_strings);
    }
    
    FILE *fout_success = fopen(output_filename, "w");
    if (!fout_success) {
        perror("Failed to open success output file");
    } else {
        size_t unique_count = 0;
        if (final_success_count > 0) {
            fprintf(fout_success, "%s\n", success_results[0]);
            unique_count++;
            for (size_t i = 1; i < final_success_count; i++) {
                if (strcmp(success_results[i], success_results[i - 1]) != 0) {
                    fprintf(fout_success, "%s\n", success_results[i]);
                    unique_count++;
                }
            }
        }
        fclose(fout_success);
        fprintf(stderr, "Successfully decoded (unique): %llu\n", (unsigned long long)unique_count);
    }

    for (size_t i = 0; i < final_success_count; i++) {
        free(success_results[i]);
    }
    free(success_results);
    
    fclose(fout_failure);
    if (fin != stdin) fclose(fin);
    free(threads);
    
    pthread_mutex_destroy(&buffer_mutex);
    pthread_mutex_destroy(&output_mutex);
    pthread_mutex_destroy(&results_mutex);
    pthread_cond_destroy(&buffer_not_full);
    pthread_cond_destroy(&buffer_not_empty);

    fprintf(stderr, "Total lines processed: %llu\n", (unsigned long long)(final_success_count + failure_count));
    fprintf(stderr, "Successfully decoded (total): %llu\n", (unsigned long long)final_success_count);
    fprintf(stderr, "Failed or non-standard: %llu\n", (unsigned long long)failure_count);
    fprintf(stderr, "Results saved to %s and %s\n", output_filename, outFileFailurePath);

    return 0;
}
