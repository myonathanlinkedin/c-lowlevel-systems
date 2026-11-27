#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define ALIGNMENT 8
#define ALIGN_UP(x) (((x) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

typedef struct block_header {
    size_t size;               // total size of the block, including header
    bool   free;               // true if block is free
    struct block_header* next; // next block in free list
} block_header_t;
