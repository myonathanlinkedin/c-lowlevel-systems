#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include "types.h"

void huff_build(const uint8_t *data, size_t len, HuffTree *tree);
size_t huff_encode(const uint8_t *data, size_t len, const HuffTree *tree,
                   uint8_t *out, size_t out_cap);
size_t huff_decode(const uint8_t *bits, size_t bit_len, const HuffTree *tree,
                   uint8_t *out, size_t out_cap);
size_t huff_bit_length(const uint8_t *data, size_t len, const HuffTree *tree);
