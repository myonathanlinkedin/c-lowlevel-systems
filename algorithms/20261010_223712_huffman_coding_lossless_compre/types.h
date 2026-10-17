
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct HuffNode {
    uint8_t symbol;
    uint32_t freq;
    struct HuffNode* left;
    struct HuffNode* right;
} HuffNode;

typedef struct {
    uint32_t bits;   // LSB-aligned code bits
    uint8_t length;  // number of valid bits (0..32)
} HuffCode;
