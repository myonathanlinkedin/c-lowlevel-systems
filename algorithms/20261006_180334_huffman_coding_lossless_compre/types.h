
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct HuffmanNode {
    uint32_t freq;
    uint8_t symbol;
    struct HuffmanNode *left;
    struct HuffmanNode *right;
    bool is_leaf;
} HuffmanNode;

typedef struct HuffmanTree {
    HuffmanNode *root;
} HuffmanTree;

typedef struct BitWriter {
    uint8_t *buffer;
    size_t capacity;
    size_t size;
    uint8_t current;
    uint8_t bits_filled;
} BitWriter;

typedef struct BitReader {
    const uint8_t *buffer;
    size_t size;
    size_t pos;
    uint8_t current;
    uint8_t bits_left;
} BitReader;

typedef struct HuffmanCode {
    uint8_t bits;
    uint8_t length;
} HuffmanCode;
