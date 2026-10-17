#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Align a size up to the nearest multiple of align (power of two). */
static size_t align_up(size_t size, size_t align) {
    assert((align & (align - 1U)) == 0U);               /* align must be power of two */
    return (size + (align - 1U)) & ~(align - 1U);
}

bool pool_init(MemoryPool *pool, size_t block_size, size_t block_count) {
    if (!pool || block_size == 0U || block_count == 0U) {
        return false;
    }

    /* Ensure each block can hold a FreeNode pointer */
    size_t min_block = sizeof(void *);
    if (block_size < min_block) {
        block_size = min_block;
    }

    /* Align block size to pointer size for safety */
    block_size = align_up(block_size, sizeof(void *));

    size_t total_bytes = block_size * block_count;
    uint8_t *buf = (uint8_t *)malloc(total_bytes);
    if (!buf) {
        return false;
    }

    /* Zero initialise for deterministic behaviour */
    memset(buf, 0, total_bytes);

    pool->block_size  = block_size;
    pool->block_count = block_count;
    pool->buffer      = buf;
    pool->free_list   = NULL;

    /* Build the free list */
    for (size_t i = 0U; i < block_count; ++i) {
        FreeNode *node = (FreeNode *)(buf + i * block_size);
        node->next = pool->free_list;
        pool->free_list = node;
    }

    return true;
}

void *pool_alloc(MemoryPool *pool) {
    if (!pool || !pool->free_list) {
        return NULL;
    }

    FreeNode *node = pool->free_list;
    pool->free_list = node->next;
    /* The returned memory is uninitialised; caller may use memset if needed */
    return (void *)node;
}

void pool_free(MemoryPool *pool, void *ptr) {
    if (!pool || !ptr) {
        return;
    }

    FreeNode *node = (FreeNode *)ptr;
    node->next = pool->free_list;
    pool->free_list = node;
}

void pool_destroy(MemoryPool *pool) {
    if (!pool) {
        return;
    }
    free(pool->buffer);
    pool->buffer = NULL;
    pool->free_list = NULL;
    pool->block_size = 0U;
    pool->block_count = 0U;
}
