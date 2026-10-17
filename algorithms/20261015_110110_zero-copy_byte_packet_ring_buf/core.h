#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#pragma once
#include "types.h"

bool rb_init(ring_buffer_t *rb, size_t capacity);
void rb_free(ring_buffer_t *rb);
bool rb_is_empty(const ring_buffer_t *rb);
bool rb_is_full(const ring_buffer_t *rb);
size_t rb_size(const ring_buffer_t *rb);
size_t rb_capacity(const ring_buffer_t *rb);
bool rb_push(ring_buffer_t *rb, uint8_t *data, size_t len);
bool rb_pop(ring_buffer_t *rb, uint8_t **data_out, size_t *len_out);
void rb_dispatch(ring_buffer_t *rb, void (*handler)(uint8_t *, size_t));
