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
 * @brief Initialise an arena with a user‑provided buffer.
 *
 * @param arena   Pointer to an Arena structure to initialise.
 * @param buffer  Pointer to a pre‑allocated memory region.
 * @param size    Size of the buffer in bytes.
 *
 * The arena does NOT take ownership of the buffer; it will not free it.
 */
void arena_init(Arena *arena, void *buffer, size_t size);

/**
 * @brief Initialise an arena by allocating its own buffer.
 *
 * @param arena   Pointer to an Arena structure to initialise.
 * @param size    Desired capacity in bytes.
 *
 * @return true on success, false on allocation failure.
 *
 * The arena takes ownership of the allocated buffer and will free it on
 * arena_destroy().
 */
bool arena_init_owned(Arena *arena, size_t size);

/**
 * @brief Allocate a block of memory from the arena.
 *
 * @param arena   Pointer to an initialised Arena.
 * @param size    Requested size in bytes.
 * @param align   Desired alignment; must be a power of two.
 *
 * @return Pointer to allocated memory, or NULL if insufficient space.
 */
void *arena_alloc(Arena *arena, size_t size, size_t align);

/**
 * @brief Reset the arena, making all previously allocated memory
 *        available again. Does not free owned buffers.
 */
void arena_reset(Arena *arena);

/**
 * @brief Destroy an arena, freeing its owned buffer if any.
 */
void arena_destroy(Arena *arena);

#ifdef __cplusplus
}
#endif
