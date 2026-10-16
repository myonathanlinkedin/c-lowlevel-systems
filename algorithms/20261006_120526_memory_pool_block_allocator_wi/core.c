#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

bool pool_init(MemoryPool *pool, size_t block_size, size_t block_count) {
    if (!pool || block_size < sizeof(void*) || block_count == 0) {
        return false;
    }

    // Align block size to pointer size for safety
    size_t align = sizeof(void*);
    if ((block_size % align) != 0) {
        block_size += align - (block_size % align);
    }

    pool->buffer = (uint8_t*)malloc(block_size * block_count);
    if (!pool->buffer) {
        return false;
    }

    pool->block_size = block_size;
    pool->block_count = block_count;
    pool->free_list = NULL;

    // Build free list: each block's first pointer stores next free block
    for (size_t i = 0; i < block_count; ++i) {
        void *block = pool->buffer + i * block_size;
        void *next = (i + 1 < block_count) ? (pool->buffer + (i + 1) * block_size) : NULL;
        memcpy(block, &next, sizeof(void*));
    }
    pool->free_list = pool->buffer; // first block is head
    return true;
}

void pool_destroy(MemoryPool *pool) {
    if (!pool) return;
    free(pool->buffer);
    pool->buffer = NULL;
    pool->free_list = NULL;
    pool->block_size = 0;
    pool->block_count = 0;
}

void *pool_alloc(MemoryPool *pool) {
    if (!pool || !pool->free_list) {
        return NULL;
    }
    // Pop head of free list
    void *block = pool->free_list;
    // The next pointer is stored at the start of the block
    void *next;
    memcpy(&next, block, sizeof(void*));
    pool->free_list = next;
    // Optionally clear the block (not required)
    return block;
}

void pool_free(MemoryPool *pool, void *block) {
    if (!pool || !block) return;
    // Push block back onto free list
    memcpy(block, &pool->free_list, sizeof(void*));
    pool->free_list = block;
}
