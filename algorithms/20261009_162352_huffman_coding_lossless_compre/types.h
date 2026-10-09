
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define HUFF_ALPHABET 256
#define HUFF_MAX_NODES (2 * HUFF_ALPHABET - 1)

typedef struct {
    uint32_t freq;
    int16_t  left;
    int16_t  right;
    int16_t  symbol;
    bool     is_leaf;
} HuffNode;

typedef struct {
    HuffNode nodes[HUFF_MAX_NODES];
    int      count;
    int      root;
    uint8_t  codes[HUFF_ALPHABET][32];
    uint8_t  lengths[HUFF_ALPHABET];
} HuffTree;
