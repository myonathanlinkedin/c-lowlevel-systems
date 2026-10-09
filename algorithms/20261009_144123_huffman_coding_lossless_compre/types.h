
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct HuffmanNode {
    uint8_t symbol;               // valid if leaf == true
    uint32_t freq;
    bool leaf;
    struct HuffmanNode *left;
    struct HuffmanNode *right;
} HuffmanNode;

typedef struct {
    uint32_t bits;   // LSB‑first representation of the code
    uint8_t  length; // number of valid bits in 'bits'
} HuffmanCode;
