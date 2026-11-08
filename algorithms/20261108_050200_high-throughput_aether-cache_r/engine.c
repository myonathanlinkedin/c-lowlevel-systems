#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdlib.h>
#include <string.h>

/* Helper: test whether a number is a power of two (and > 0). */
static bool is_power_of_two(size_t x) {
    return x != 0 && (x & (x - 1)) == 0;
}

bool aether_ring_init(aether_ring_t *rb, size_t capacity) {
    if (!rb || !is_power_of_two(capacity)) {
        return false;
    }

    rb->buffer = (uint8_t *)malloc(capacity * sizeof(uint8_t));
    if (!rb->buffer) {
        return false;
    }

    rb->capacity = capacity;
    rb->mask = capacity - 1;
    atomic_store_explicit(&rb->head, 0, memory_order_relaxed);
    atomic_store_explicit(&rb->tail, 0, memory_order_relaxed);
    return true;
}

void aether_ring_free(aether_ring_t *rb) {
    if (rb && rb->buffer) {
        free(rb->buffer);
        rb->buffer = NULL;
        rb->capacity = 0;
        rb->mask = 0;
        atomic_store_explicit(&rb->head, 0, memory_order_relaxed);
        atomic_store_explicit(&rb->tail, 0, memory_order_relaxed);
    }
}

bool aether_ring_is_full(const aether_ring_t *rb) {
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    return ((head + 1) & rb->mask) == tail;
}

bool aether_ring_is_empty(const aether_ring_t *rb) {
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    return head == tail;
}

bool aether_ring_push(aether_ring_t *rb, uint8_t value) {
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    size_t next = (head + 1) & rb->mask;

    if (next == tail) {
        /* Buffer full */
        return false;
    }

    rb->buffer[head] = value;
    atomic_store_explicit(&rb->head, next, memory_order_release);
    return true;
}

bool aether_ring_pop(aether_ring_t *rb, uint8_t *value) {
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);

    if (head == tail) {
        /* Buffer empty */
        return false;
    }

    *value = rb->buffer[tail];
    size_t next = (tail + 1) & rb->mask;
    atomic_store_explicit(&rb->tail, next, memory_order_release);
    return true;
}

size_t aether_ring_size(const aether_ring_t *rb) {
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    return (head - tail) & rb->mask;
}
