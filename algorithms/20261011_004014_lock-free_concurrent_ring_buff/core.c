#include "types.h"
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Helper: round up to next power of two (returns 0 for input 0). */
static size_t next_pow2(size_t v) {
    if (v == 0) return 0;
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

bool ring_init(ring_buffer_t *rb, size_t capacity) {
    if (!rb) return false;
    size_t cap = next_pow2(capacity);
    if (cap == 0) return false;
    int *buf = (int *)malloc(cap * sizeof(int));
    if (!buf) return false;
    rb->capacity = cap;
    rb->mask = cap - 1;
    atomic_init(&rb->head, 0);
    atomic_init(&rb->tail, 0);
    rb->buffer = buf;
    return true;
}

void ring_free(ring_buffer_t *rb) {
    if (rb && rb->buffer) {
        free(rb->buffer);
        rb->buffer = NULL;
        rb->capacity = 0;
        rb->mask = 0;
        atomic_store_explicit(&rb->head, 0, memory_order_relaxed);
        atomic_store_explicit(&rb->tail, 0, memory_order_relaxed);
    }
}

bool ring_push(ring_buffer_t *rb, int value) {
    assert(rb);
    size_t head = atomic_load_explicit(&rb->head, memory_order_relaxed);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    if ((head - tail) == rb->capacity) {
        /* Buffer full */
        return false;
    }
    rb->buffer[head & rb->mask] = value;
    atomic_store_explicit(&rb->head, head + 1, memory_order_release);
    return true;
}

bool ring_pop(ring_buffer_t *rb, int *value) {
    assert(rb && value);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_relaxed);
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    if (head == tail) {
        /* Buffer empty */
        return false;
    }
    *value = rb->buffer[tail & rb->mask];
    atomic_store_explicit(&rb->tail, tail + 1, memory_order_release);
    return true;
}
