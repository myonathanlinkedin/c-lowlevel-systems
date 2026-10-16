#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize a memory pool.
 *
 * @param pool        Pointer to MemoryPool structure to initialise.
 * @param block_size  Size of each block in bytes. Must be at least sizeof(void*).
 * @param block_count Number of blocks to allocate.
 * @return true on success, false on allocation failure or invalid parameters.
 */
bool pool_init(MemoryPool *pool, size_t block_size, size_t block_count);

/**
 * @brief Release resources owned by the pool.
 *
 * @param pool Pointer to MemoryPool.
 */
void pool_destroy(MemoryPool *pool);

/**
 * @brief Allocate a block from the pool.
 *
 * @param pool Pointer to MemoryPool.
 * @return Pointer to a block, or NULL if no blocks are available.
 */
void *pool_alloc(MemoryPool *pool);

/**
 * @brief Return a previously allocated block to the pool.
 *
 * @param pool  Pointer to MemoryPool.
 * @param block Pointer returned by pool_alloc.
 */
void pool_free(MemoryPool *pool, void *block);

#ifdef __cplusplus
}
#endif
