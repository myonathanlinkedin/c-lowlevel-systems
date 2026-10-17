#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/* MemoryPool: a fixed‑size block allocator using a singly linked free list.
 *
 * The pool owns a contiguous buffer of `capacity * block_size` bytes.
 * Each free block stores a pointer to the next free block in its first
 * sizeof(void*) bytes.  Allocated blocks are handed to the caller without
 * any additional header.
 */
typedef struct MemoryPool {
    uint8_t *buffer;      /* raw memory buffer */
    void *free_list;      /* head of singly linked free list */
    size_t block_size;    /* size of each block (must be >= sizeof(void*)) */
    size_t capacity;      /* total number of blocks */
    size_t used;          /* number of blocks currently allocated */
} MemoryPool;

/* Initialise a memory pool.
 *   pool        : pointer to an uninitialised MemoryPool structure.
 *   block_size  : size of each block; must be at least sizeof(void*).
 *   capacity    : number of blocks to allocate.
 * Returns true on success, false on allocation failure or invalid parameters.
 */
bool mpool_init(MemoryPool *pool, size_t block_size, size_t capacity);

/* Allocate a single block from the pool.
 * Returns a pointer to a usable block, or NULL if the pool is exhausted.
 */
void *mpool_alloc(MemoryPool *pool);

/* Return a previously allocated block to the pool.
 *   ptr may be NULL (no‑op).  Behaviour is undefined if ptr does not belong
 *   to the pool or was already freed.
 */
void mpool_free(MemoryPool *pool, void *ptr);

/* Release all resources owned by the pool.
 * After this call the pool must not be used unless re‑initialised.
 */
void mpool_destroy(MemoryPool *pool);
