#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#pragma once
#include "types.h"

HuffmanTree *build_huffman_tree(const uint8_t *data, size_t len);
void free_huffman_tree(HuffmanTree *tree);
void generate_codes(const HuffmanNode *node, HuffmanCode *codes, uint8_t code, uint8_t length);
bool encode(const uint8_t *data, size_t len, const HuffmanCode *codes, BitWriter *out);
bool decode(const BitReader *in, const HuffmanTree *tree, uint8_t *out, size_t *out_len);
