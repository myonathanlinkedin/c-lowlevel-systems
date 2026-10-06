#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t offset; // offset from arena base
} CompactPtr;

typedef struct {
    uint8_t *base;
    size_t capacity;
    size_t used;
} CompactArena;
