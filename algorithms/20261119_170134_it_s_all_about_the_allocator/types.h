#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct BlockHeader {
    size_t size;                 // Size of the user payload (excluding header)
    bool   free;                 // Allocation status
    struct BlockHeader* next;    // Next block in memory order
    struct BlockHeader* prev;    // Previous block in memory order
    struct BlockHeader* next_free; // Next free block in free list
    struct BlockHeader* prev_free; // Prev free block in free list
} BlockHeader;

// Align to 8 bytes (must be power of two)
#define ALIGNMENT ((size_t)8)
#define ALIGN_UP(sz) (((sz) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
