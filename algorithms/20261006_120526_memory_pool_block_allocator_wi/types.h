#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct MemoryPool {
    uint8_t *buffer;          // Raw memory buffer
    size_t block_size;        // Size of each block (must be >= sizeof(void*))
    size_t block_count;       // Number of blocks in the pool
    void *free_list;          // Head of singly linked free list
} MemoryPool;
