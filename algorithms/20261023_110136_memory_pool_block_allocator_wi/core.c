#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Helper: round up to the nearest multiple of align (power of two). */
static size_t round_up_pow2(size_t size, size_t align) {
    assert((align & (align - 1)) == 0);          // align must be power of two
    return (size + align - 1) & ~(align - 1);
}

/* Helper: compute pointer offset as size_t */
static size_t ptr_offset(const void *base, const void *ptr) {
    return (const unsigned char *)ptr - (const unsigned char *)base;
}

/* Initialise the pool */
bool mpool_init(MemoryPool *pool, size_t block_size, size_t block_count) {
    if (!pool || block_size == 0 || block_count == 0) {
        return false;
    }

    /* Ensure each block can store a next pointer */
    size_t min_align = sizeof(void *);
    size_t aligned = round_up_pow2(block_size, min_align);
    if (aligned < min_align) {   // overflow guard
        return false;
    }

    size_t total_bytes = aligned * block_count;
    if (total_bytes / aligned != block_count) {   // overflow guard
        return false;
    }

    void *buf = malloc(total_bytes);
    if (!buf) {
        return false;
    }

    /* Build free list */
    unsigned char *p = (unsigned char *)buf;
    for (size_t i = 0; i < block_count; ++i) {
        void **next_ptr = (void **)p;
        if (i + 1 < block_count) {
            *next_ptr = (void *)(p + aligned);
        } else {
            *next_ptr = NULL;
        }
        p += aligned;
    }

    pool->block_size   = aligned;
    pool->total_blocks = block_count;
    pool->buffer       = buf;
    pool->free_list    = buf;
    return true;
}

/* Allocate a block */
void *mpool_alloc(MemoryPool *pool) {
    if (!pool || !pool->free_list) {
        return NULL;
    }
    void *block = pool->free_list;
    pool->free_list = *(void **)block;   // next free block
    return block;
}

/* Verify that ptr belongs to pool and is correctly aligned */
static bool ptr_in_pool(const MemoryPool *pool, const void *ptr) {
    if (!pool || !ptr) {
        return false;
    }
    size_t offset = ptr_offset(pool->buffer, ptr);
    if (offset % pool->block_size != 0) {
        return false;
    }
    if (offset >= pool->block_size * pool->total_blocks) {
        return false;
    }
    return true;
}

/* Detect double free by scanning free list (O(n), acceptable for test). */
static bool is_already_free(const MemoryPool *pool, const void *ptr) {
    for (void *cur = pool->free_list; cur != NULL; cur = *(void **)cur) {
        if (cur == ptr) {
            return true;
        }
    }
    return false;
}

/* Free a block */
bool mpool_free(MemoryPool *pool, void *ptr) {
    if (!ptr_in_pool(pool, ptr)) {
        return false;
    }
    if (is_already_free(pool, ptr)) {
        return false;   // double free
    }
    /* Push back onto free list */
    *(void **)ptr = pool->free_list;
    pool->free_list = ptr;
    return true;
}

/* Destroy the pool */
void mpool_destroy(MemoryPool *pool) {
    if (!pool) {
        return;
    }
    free(pool->buffer);
    /* Zero out fields to avoid accidental reuse */
    pool->buffer = NULL;
    pool->free_list = NULL;
    pool->block_size = 0;
    pool->total_blocks = 0;
}
