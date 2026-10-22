#include "types.h"
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>

/* Helper: check if a number is a power of two */
static inline bool is_power_of_two(size_t x) {
    return x != 0 && (x & (x - 1)) == 0;
}

/* Helper: round up to next power of two (used only for defensive programming) */
static size_t next_power_of_two(size_t x) {
    if (x == 0) return 1;
    --x;
    for (size_t i = 1; i < sizeof(size_t) * 8; i <<= 1)
        x |= x >> i;
    return x + 1;
}

bool crb_init(ChronosRingBuffer *rb, size_t capacity) {
    if (!rb) return false;
    if (!is_power_of_two(capacity)) {
        /* For robustness we round up, but signal failure to caller */
        capacity = next_power_of_two(capacity);
        return false;
    }
    rb->capacity = capacity;
    rb->mask = capacity - 1;
    atomic_store_explicit(&rb->head, 0, memory_order_relaxed);
    atomic_store_explicit(&rb->tail, 0, memory_order_relaxed);
    rb->buffer = (void **)malloc(sizeof(void *) * capacity);
    if (!rb->buffer) return false;
    memset(rb->buffer, 0, sizeof(void *) * capacity);
    return true;
}

void crb_free(ChronosRingBuffer *rb) {
    if (!rb) return;
    free(rb->buffer);
    rb->buffer = NULL;
    rb->capacity = 0;
    rb->mask = 0;
    atomic_store_explicit(&rb->head, 0, memory_order_relaxed);
    atomic_store_explicit(&rb->tail, 0, memory_order_relaxed);
}

/* Single‑producer push */
bool crb_push(ChronosRingBuffer *rb, void *item) {
    if (!rb || !item) return false;
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_relaxed);
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    if (((tail + 1) & rb->mask) == (head & rb->mask)) {
        /* Buffer full */
        return false;
    }
    rb->buffer[tail & rb->mask] = item;
    atomic_store_explicit(&rb->tail, tail + 1, memory_order_release);
    return true;
}

/* Single‑consumer pop */
bool crb_pop(ChronosRingBuffer *rb, void **item_out) {
    if (!rb || !item_out) return false;
    size_t head = atomic_load_explicit(&rb->head, memory_order_relaxed);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    if ((head & rb->mask) == (tail & rb->mask)) {
        /* Buffer empty */
        return false;
    }
    *item_out = rb->buffer[head & rb->mask];
    /* Optional: clear slot for debugging */
    rb->buffer[head & rb->mask] = NULL;
    atomic_store_explicit(&rb->head, head + 1, memory_order_release);
    return true;
}

size_t crb_size(const ChronosRingBuffer *rb) {
    if (!rb) return 0;
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    return (tail - head) & rb->mask;
}

bool crb_is_full(const ChronosRingBuffer *rb) {
    if (!rb) return false;
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    return ((tail + 1) & rb->mask) == (head & rb->mask);
}

bool crb_is_empty(const ChronosRingBuffer *rb) {
    if (!rb) return true;
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    return (head & rb->mask) == (tail & rb->mask);
}
