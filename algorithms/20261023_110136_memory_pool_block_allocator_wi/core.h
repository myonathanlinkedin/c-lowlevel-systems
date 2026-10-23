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
 * @brief Initialise a memory pool.
 *
 * @param pool          Pointer to an uninitialised MemoryPool structure.
 * @param block_size    Desired size of each block (bytes). Must be >0.
 * @param block_count   Number of blocks to allocate.
 * @return true         On successful allocation and initialisation.
 * @return false        On failure (e.g., zero sizes or allocation error).
 *
 * The function rounds block_size up to at least sizeof(void*) to store the free‑list pointer.
 */
bool mpool_init(MemoryPool *pool, size_t block_size, size_t block_count);

/**
 * @brief Allocate a single block from the pool.
 *
 * @param pool  Pointer to an initialised MemoryPool.
 * @return void* Pointer to a usable block, or NULL if the pool is exhausted.
 */
void *mpool_alloc(MemoryPool *pool);

/**
 * @brief Return a previously allocated block to the pool.
 *
 * @param pool  Pointer to an initialised MemoryPool.
 * @param ptr   Pointer previously obtained from mpool_alloc.
 * @return true  If the block was successfully returned.
 * @return false If ptr is invalid, out of range, mis‑aligned, or already freed.
 */
bool mpool_free(MemoryPool *pool, void *ptr);

/**
 * @brief Release all resources owned by the pool.
 *
 * After this call the MemoryPool structure must not be used unless re‑initialised.
 */
void mpool_destroy(MemoryPool *pool);

#ifdef __cplusplus
}
#endif
