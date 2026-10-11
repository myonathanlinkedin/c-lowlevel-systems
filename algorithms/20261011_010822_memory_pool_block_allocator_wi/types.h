
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Forward declaration for free list node */
typedef struct FreeNode {
    struct FreeNode *next;
} FreeNode;

/* Memory pool structure */
typedef struct {
    size_t block_size;   /* Size of each block (in bytes) */
    size_t block_count;  /* Total number of blocks */
    uint8_t *buffer;     /* Raw memory buffer */
    FreeNode *free_list; /* Head of singly linked free list */
} MemoryPool;

/* Public API */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialise a memory pool.
 *
 * @param pool        Pointer to MemoryPool structure (must be valid).
 * @param block_size  Desired size of each block (bytes). Must be >= sizeof(void*).
 * @param block_count Number of blocks to allocate.
 *
 * @return true on success, false on allocation failure or invalid parameters.
 */
bool pool_init(MemoryPool *pool, size_t block_size, size_t block_count);

/**
 * @brief Allocate a single block from the pool.
 *
 * @param pool Pointer to an initialised MemoryPool.
 *
 * @return Pointer to a usable block, or NULL if the pool is exhausted.
 */
void *pool_alloc(MemoryPool *pool);

/**
 * @brief Return a previously allocated block to the pool.
 *
 * @param pool Pointer to an initialised MemoryPool.
 * @param ptr  Pointer previously obtained from pool_alloc. Passing NULL is a no‑op.
 *
 * @note Behaviour is undefined if ptr does not belong to the pool.
 */
void pool_free(MemoryPool *pool, void *ptr);

/**
 * @brief Release all resources owned by the pool.
 *
 * @param pool Pointer to an initialised MemoryPool.
 *
 * After this call the pool must not be used unless re‑initialised.
 */
void pool_destroy(MemoryPool *pool);

#ifdef __cplusplus
}
#endif
