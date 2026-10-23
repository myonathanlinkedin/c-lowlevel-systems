#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#pragma once
#include <stddef.h>
#include <stdbool.h>

typedef struct MemoryPool {
    size_t block_size;      // Size of each block (rounded up to align)
    size_t total_blocks;    // Number of blocks in the pool
    void *buffer;           // Raw memory buffer containing all blocks
    void *free_list;        // Head of singly‑linked free list (stores next pointer in each free block)
} MemoryPool;
