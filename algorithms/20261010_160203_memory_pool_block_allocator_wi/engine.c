#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* Helper: compute the aligned block size (must be at least sizeof(void*)).
 * Alignment is to the natural pointer size to guarantee that the stored
 * free‑list pointer is correctly aligned.
 */
static size_t aligned_block_size(size_t requested) {
    size_t alignment = sizeof(void *);
    size_t remainder = requested % alignment;
    return remainder == 0 ? requested : requested + (alignment - remainder);
}

/* Initialise the pool. */
bool mpool_init(MemoryPool *pool, size_t block_size, size_t capacity) {
    if (!pool || capacity == 0) {
        return false;
    }

    /* Ensure block size can hold a pointer for the free‑list link. */
    size_t actual_block = aligned_block_size(block_size);
    if (actual_block < sizeof(void *)) {
        actual_block = sizeof(void *);
    }

    size_t total_bytes = actual_block * capacity;
    uint8_t *buf = (uint8_t *)malloc(total_bytes);
    if (!buf) {
        return false;
    }

    /* Build the free list: each block's first bytes store the next pointer. */
    void *head = NULL;
    for (size_t i = 0; i < capacity; ++i) {
        void *block = buf + i * actual_block;
        *(void **)block = head;
        head = block;
    }

    pool->buffer = buf;
    pool->free_list = head;
    pool->block_size = actual_block;
    pool->capacity = capacity;
    pool->used = 0;
    return true;
}

/* Allocate a block. */
void *mpool_alloc(MemoryPool *pool) {
    assert(pool);
    if (!pool->free_list) {
        return NULL; /* pool exhausted */
    }

    /* Pop the head of the free list. */
    void *block = pool->free_list;
    pool->free_list = *(void **)block;
    ++pool->used;
    return block;
}

/* Return a block to the pool. */
void mpool_free(MemoryPool *pool, void *ptr) {
    if (!pool || !ptr) {
        return; /* no‑op for NULL */
    }

    /* Simple sanity check: ptr must lie inside the pool buffer. */
    uint8_t *byte_ptr = (uint8_t *)ptr;
    if (byte_ptr < pool->buffer ||
        byte_ptr >= pool->buffer + pool->block_size * pool->capacity) {
        /* Out‑of‑bounds free – undefined behaviour, but we assert in debug. */
        assert(!"mpool_free: pointer does not belong to pool");
        return;
    }

    /* Push the block back onto the free list. */
    *(void **)ptr = pool->free_list;
    pool->free_list = ptr;
    if (pool->used > 0) {
        --pool->used;
    }
}

/* Destroy the pool and release memory. */
void mpool_destroy(MemoryPool *pool) {
    if (!pool) {
        return;
    }
    free(pool->buffer);
    /* Zero out fields to catch accidental reuse. */
    pool->buffer = NULL;
    pool->free_list = NULL;
    pool->block_size = 0;
    pool->capacity = 0;
    pool->used = 0;
}
