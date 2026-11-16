#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

bool pool_init(Pool *pool, size_t block_size, size_t block_count) {
    if (!pool || block_size == 0 || block_count == 0) {
        return false;
    }

    const size_t alignment = alignof(max_align_t);
    const size_t aligned_block_size = (block_size + alignment - 1) & ~(alignment - 1);
    const size_t total_size = aligned_block_size * block_count;

    void *mem = aligned_alloc(alignment, total_size);
    if (!mem) {
        return false;
    }

    // Initialize free list
    Block *current = (Block *)mem;
    Block *prev = NULL;
    for (size_t i = 0; i < block_count; ++i) {
        current->next = prev;
        prev = current;
        current = (Block *)((char *)current + aligned_block_size);
    }

    pool->block_size = aligned_block_size;
    pool->block_count = block_count;
    pool->memory = mem;
    pool->free_list = prev;

    return true;
}

void *pool_alloc(Pool *pool) {
    if (!pool || !pool->free_list) {
        return NULL;
    }

    Block *block = pool->free_list;
    pool->free_list = block->next;
    return (void *)block;
}

void pool_free(Pool *pool, void *ptr) {
    if (!pool || !ptr) {
        return;
    }

    Block *block = (Block *)ptr;
    block->next = pool->free_list;
    pool->free_list = block;
}

void pool_destroy(Pool *pool) {
    if (!pool) {
        return;
    }

    free(pool->memory);
    pool->memory = NULL;
    pool->free_list = NULL;
    pool->block_size = 0;
    pool->block_count = 0;
}
