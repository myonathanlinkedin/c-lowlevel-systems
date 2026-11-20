#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#pragma once
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Ring buffer structure
typedef struct {
    size_t capacity;          // must be power of two
    size_t mask;              // capacity - 1
    _Atomic size_t head;      // write index
    _Atomic size_t tail;      // read index
    aether_item_t *buffer;    // array of items
} aether_ring_t;

// Initialise a ring buffer; returns true on success, false on allocation failure.
// If capacity is not a power of two, it is rounded up to the next power of two.
bool aether_ring_init(aether_ring_t *rb, size_t capacity);

// Release resources associated with a ring buffer.
void aether_ring_destroy(aether_ring_t *rb);

// Push an item onto the ring; returns false if the buffer is full.
bool aether_ring_push(aether_ring_t *rb, aether_item_t item);

// Pop an item from the ring; returns false if the buffer is empty.
bool aether_ring_pop(aether_ring_t *rb, aether_item_t *item_out);

// Current number of stored items.
size_t aether_ring_size(const aether_ring_t *rb);

// Capacity (maximum number of items that can be stored).
static inline size_t aether_ring_capacity(const aether_ring_t *rb) { return rb->capacity; }

// Test helpers
static inline bool aether_ring_is_empty(const aether_ring_t *rb) { return aether_ring_size(rb) == 0; }
static inline bool aether_ring_is_full(const aether_ring_t *rb)  { return aether_ring_size(rb) == rb->capacity; }

#ifdef __cplusplus
}
#endif
