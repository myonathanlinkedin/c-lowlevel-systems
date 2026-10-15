#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <assert.h>

static size_t next_pow2(size_t v) {
    if (v == 0) return 1;
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
#if UINT_MAX > 0xFFFFFFFF
    v |= v >> 32;
#endif
    v++;
    return v;
}

bool rb_init(ring_buffer_t *rb, size_t capacity) {
    if (rb == NULL) return false;
    size_t cap = next_pow2(capacity);
    rb->buf = (packet_t *)malloc(cap * sizeof(packet_t));
    if (rb->buf == NULL) return false;
    rb->capacity = cap;
    rb->mask = cap - 1;
    rb->head = 0;
    rb->tail = 0;
    return true;
}

void rb_free(ring_buffer_t *rb) {
    if (rb && rb->buf) {
        free(rb->buf);
        rb->buf = NULL;
        rb->capacity = 0;
        rb->mask = 0;
        rb->head = rb->tail = 0;
    }
}

bool rb_is_empty(const ring_buffer_t *rb) {
    return rb->head == rb->tail;
}

bool rb_is_full(const ring_buffer_t *rb) {
    return ((rb->tail - rb->head) & rb->mask) == rb->mask;
}

size_t rb_size(const ring_buffer_t *rb) {
    return (rb->tail - rb->head) & rb->mask;
}

size_t rb_capacity(const ring_buffer_t *rb) {
    return rb->capacity;
}

bool rb_push(ring_buffer_t *rb, uint8_t *data, size_t len) {
    if (rb_is_full(rb)) return false;
    size_t idx = rb->tail & rb->mask;
    rb->buf[idx].data = data;
    rb->buf[idx].len = len;
    rb->tail++;
    return true;
}

bool rb_pop(ring_buffer_t *rb, uint8_t **data_out, size_t *len_out) {
    if (rb_is_empty(rb)) return false;
    size_t idx = rb->head & rb->mask;
    if (data_out) *data_out = rb->buf[idx].data;
    if (len_out)  *len_out  = rb->buf[idx].len;
    rb->head++;
    return true;
}

void rb_dispatch(ring_buffer_t *rb, void (*handler)(uint8_t *, size_t)) {
    assert(handler != NULL);
    uint8_t *data;
    size_t len;
    while (rb_pop(rb, &data, &len)) {
        handler(data, len);
    }
}
