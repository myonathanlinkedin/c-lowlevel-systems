#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#pragma once
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initialise the allocator with a pre‑allocated memory pool. */
void allocator_init(void* pool, size_t size);

/* Allocate a block of at least `size` bytes. Returns NULL on failure. */
void* allocator_alloc(size_t size);

/* Return a previously allocated block to the allocator. */
void allocator_free(void* ptr);

/* Optional diagnostic dump (useful during development). */
void allocator_dump(void);

#ifdef __cplusplus
}
#endif
