#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>

typedef struct Block {
    struct Block *next;
} Block;

typedef struct Pool {
    size_t block_size;   // Size of each block (aligned)
    size_t block_count;  // Number of blocks in the pool
    void *memory;        // Pointer to the raw memory block
    Block *free_list;    // Head of the free list
} Pool;

// Function prototypes
bool pool_init(Pool *pool, size_t block_size, size_t block_count);
void *pool_alloc(Pool *pool);
void pool_free(Pool *pool, void *ptr);
void pool_destroy(Pool *pool);
