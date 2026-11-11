#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#pragma once

#include "types.h"

/* Initialize a ring buffer.
 *  capacity must be a power of two and > 0.
 *  Returns true on success, false on allocation failure or invalid capacity.
 */
bool ring_init(ring_t *r, size_t capacity);

/* Release resources held by the ring buffer. */
void ring_free(ring_t *r);

/* Push an element into the ring.
 *  Returns true on success, false if the buffer is full.
 */
bool ring_push(ring_t *r, void *item);

/* Pop an element from the ring.
 *  Returns true on success and stores the element in *out,
 *  false if the buffer is empty.
 */
bool ring_pop(ring_t *r, void **out);

/* Query functions. */
bool ring_is_empty(const ring_t *r);
bool ring_is_full(const ring_t *r);

/* Utility: check if a number is power of two. */
static inline bool is_power_of_two(size_t x) {
    return x != 0 && (x & (x - 1)) == 0;
}
