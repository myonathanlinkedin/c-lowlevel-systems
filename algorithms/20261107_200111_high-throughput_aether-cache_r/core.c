#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include "core.h"
#include <stdlib.h>
#include <assert.h>

static inline bool is_power_of_two(size_t x) {
    return x && ((x & (x - 1)) == 0);
}

bool ring_init(ring_t *r, size_t capacity) {
    if (!r || capacity == 0 || !is_power_of_two(capacity)) {
        return false;
    }
    r->capacity = capacity;
    r->mask = capacity - 1;
    r->buffer = (void **)calloc(capacity, sizeof(void *));
    if (!r->buffer) {
        return false;
    }
    atomic_init(&r->head, 0);
    atomic_init(&r->tail, 0);
    return true;
}

void ring_free(ring_t *r) {
    if (!r) return;
    free(r->buffer);
    r->buffer = NULL;
    r->capacity = 0;
    r->mask = 0;
    atomic_store(&r->head, 0);
    atomic_store(&r->tail, 0);
}

bool ring_is_empty(const ring_t *r) {
    return atomic_load_explicit(&r->head, memory_order_acquire) ==
           atomic_load_explicit(&r->tail, memory_order_acquire);
}

bool ring_is_full(const ring_t *r) {
    size_t head = atomic_load_explicit(&r->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&r->tail, memory_order_acquire);
    return ((head + 1) & r->mask) == (tail & r->mask);
}

bool ring_push(ring_t *r, void *item) {
    size_t head = atomic_load_explicit(&r->head, memory_order_relaxed);
    size_t next_head = (head + 1) & r->mask;
    size_t tail = atomic_load_explicit(&r->tail, memory_order_acquire);
    if (next_head == (tail & r->mask)) {
        return false; // buffer full
    }
    r->buffer[head] = item;
    atomic_store_explicit(&r->head, next_head, memory_order_release);
    return true;
}

bool ring_pop(ring_t *r, void **item) {
    size_t tail = atomic_load_explicit(&r->tail, memory_order_relaxed);
    size_t head = atomic_load_explicit(&r->head, memory_order_acquire);
    if (tail == (head & r->mask)) {
        return false; // buffer empty
    }
    *item = r->buffer[tail];
    size_t next_tail = (tail + 1) & r->mask;
    atomic_store_explicit(&r->tail, next_tail, memory_order_release);
    return true;
}
