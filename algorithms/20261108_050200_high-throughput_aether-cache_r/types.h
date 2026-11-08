
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>

/* Aether-Cache Ring Buffer
 *
 * The buffer size must be a power of two; this enables fast index wrapping
 * using a bitmask. The structure is designed for single‑producer / single‑consumer
 * usage without locks, relying on C11 atomic operations.
 */
typedef struct {
    uint8_t *buffer;          /* Dynamically allocated storage */
    size_t   capacity;        /* Must be a power of two */
    size_t   mask;            /* capacity - 1, for fast modulo */
    _Atomic size_t head;      /* Write index (producer) */
    _Atomic size_t tail;      /* Read index  (consumer) */
} aether_ring_t;

/* Initialise a ring buffer.
 * Returns true on success, false if capacity is not a power of two or allocation fails.
 */
bool aether_ring_init(aether_ring_t *rb, size_t capacity);

/* Release resources held by the ring buffer. */
void aether_ring_free(aether_ring_t *rb);

/* Push a byte into the buffer.
 * Returns true if the value was stored, false if the buffer is full.
 */
bool aether_ring_push(aether_ring_t *rb, uint8_t value);

/* Pop a byte from the buffer.
 * Returns true if a value was retrieved, false if the buffer is empty.
 */
bool aether_ring_pop(aether_ring_t *rb, uint8_t *value);

/* Query the current number of stored elements. */
size_t aether_ring_size(const aether_ring_t *rb);

/* Convenience predicates. */
bool aether_ring_is_empty(const aether_ring_t *rb);
bool aether_ring_is_full(const aether_ring_t *rb);
