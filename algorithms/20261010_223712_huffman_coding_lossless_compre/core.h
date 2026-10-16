#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#pragma once

#include "types.h"

/* Compresses `input_len` bytes from `input` into a newly allocated buffer.
 * The function stores the size of the compressed data in `*output_len`
 * and returns a pointer to the buffer via `*output`. The caller must free
 * the buffer with `huffman_free()`. Returns true on success, false on OOM. */
bool huffman_compress(const uint8_t* input, size_t input_len,
                      uint8_t** output, size_t* output_len);

/* Decompresses a buffer produced by `huffman_compress`.
 * The original size is written to `*output_len` and the data buffer
 * is returned via `*output`. The caller must free the buffer with
 * `huffman_free()`. Returns true on success, false on malformed input
 * or OOM. */
bool huffman_decompress(const uint8_t* input, size_t input_len,
                        uint8_t** output, size_t* output_len);

/* Frees a buffer allocated by the library. */
void huffman_free(uint8_t* buffer);
