#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct Arena {
    uint8_t *buffer;      // base pointer
    size_t   capacity;    // total size of buffer in bytes
    size_t   offset;      // current used offset
    bool     owned;       // true if buffer was allocated by arena_init
} Arena;
