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

// Initialize allocator with a memory pool of at least `pool_size` bytes.
// Returns true on success, false on allocation failure.
bool allocator_init(size_t pool_size);

// Allocate `size` bytes. Returns NULL on failure.
void* allocator_alloc(size_t size);

// Free a previously allocated pointer. Undefined behavior if ptr is invalid.
void allocator_free(void* ptr);

// Release the internal memory pool. After this call the allocator must be re‑initialized.
void allocator_destroy(void);

#ifdef __cplusplus
}
#endif
