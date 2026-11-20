#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Helper: round up to next power of two (returns 1 for 0 input)
static size_t round_up_pow2(size_t v) {
    if (v == 0) return 1;
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
#if SIZE_MAX > UINT32_MAX
    v |= v >> 32;
#endif
    return v + 1;
}

bool aether_ring_init(aether_ring_t *rb, size_t capacity) {
    if (!rb) return false;
    size_t cap = round_up_pow2(capacity);
    aether_item_t *buf = (aether_item_t *)malloc(sizeof(aether_item_t) * cap);
    if (!buf) return false;
    rb->capacity = cap;
    rb->mask = cap - 1;
    atomic_store_explicit(&rb->head, 0, memory_order_relaxed);
    atomic_store_explicit(&rb->tail, 0, memory_order_relaxed);
    rb->buffer = buf;
    return true;
}

void aether_ring_destroy(aether_ring_t *rb) {
    if (!rb) return;
    free(rb->buffer);
    rb->buffer = NULL;
    rb->capacity = 0;
    rb->mask = 0;
    atomic_store_explicit(&rb->head, 0, memory_order_relaxed);
    atomic_store_explicit(&rb->tail, 0, memory_order_relaxed);
}

bool aether_ring_push(aether_ring_t *rb, aether_item_t item) {
    assert(rb);
    size_t head = atomic_load_explicit(&rb->head, memory_order_relaxed);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    if ((head - tail) == rb->capacity) {
        // buffer full
        return false;
    }
    rb->buffer[head & rb->mask] = item;
    // Ensure the store to buffer happens before the head update
    atomic_store_explicit(&rb->head, head + 1, memory_order_release);
    return true;
}

bool aether_ring_pop(aether_ring_t *rb, aether_item_t *item_out) {
    assert(rb && item_out);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_relaxed);
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    if (head == tail) {
        // buffer empty
        return false;
    }
    *item_out = rb->buffer[tail & rb->mask];
    // Ensure the load from buffer happens before the tail update
    atomic_store_explicit(&rb->tail, tail + 1, memory_order_release);
    return true;
}

size_t aether_ring_size(const aether_ring_t *rb) {
    assert(rb);
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    return head - tail;
}
