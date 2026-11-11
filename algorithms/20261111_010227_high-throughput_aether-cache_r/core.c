#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <assert.h>

/* Allocate and initialise the ring buffer.
 * Time: O(1) amortised, Space: O(capacity)
 */
bool ring_init(ring_t *r, size_t capacity) {
    if (!is_power_of_two(capacity)) {
        return false;
    }
    r->buffer = (void **)malloc(sizeof(void *) * capacity);
    if (!r->buffer) {
        return false;
    }
    r->capacity = capacity;
    r->mask = capacity - 1;
    atomic_store_explicit(&r->head, 0, memory_order_relaxed);
    atomic_store_explicit(&r->tail, 0, memory_order_relaxed);
    return true;
}

/* Release the allocated storage. */
void ring_free(ring_t *r) {
    free(r->buffer);
    r->buffer = NULL;
    r->capacity = 0;
    r->mask = 0;
    atomic_store_explicit(&r->head, 0, memory_order_relaxed);
    atomic_store_explicit(&r->tail, 0, memory_order_relaxed);
}

/* Return true if buffer is full.
 * Full when (tail - head) == capacity.
 */
bool ring_is_full(const ring_t *r) {
    size_t head = atomic_load_explicit(&r->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&r->tail, memory_order_acquire);
    return (tail - head) == r->capacity;
}

/* Return true if buffer is empty. */
bool ring_is_empty(const ring_t *r) {
    size_t head = atomic_load_explicit(&r->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&r->tail, memory_order_acquire);
    return head == tail;
}

/* Push an item; fails if full.
 * Producer only touches tail, reads head.
 */
bool ring_push(ring_t *r, void *item) {
    size_t tail = atomic_load_explicit(&r->tail, memory_order_relaxed);
    size_t head = atomic_load_explicit(&r->head, memory_order_acquire);
    if ((tail - head) == r->capacity) {
        return false; // full
    }
    r->buffer[tail & r->mask] = item;
    atomic_store_explicit(&r->tail, tail + 1, memory_order_release);
    return true;
}

/* Pop an item; fails if empty.
 * Consumer only touches head, reads tail.
 */
bool ring_pop(ring_t *r, void **out) {
    size_t head = atomic_load_explicit(&r->head, memory_order_relaxed);
    size_t tail = atomic_load_explicit(&r->tail, memory_order_acquire);
    if (head == tail) {
        return false; // empty
    }
    *out = r->buffer[head & r->mask];
    atomic_store_explicit(&r->head, head + 1, memory_order_release);
    return true;
}
