#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#pragma once
#include "types.h"

bool ring_init(ring_t *r, size_t capacity);
void ring_free(ring_t *r);
bool ring_push(ring_t *r, void *item);
bool ring_pop(ring_t *r, void **item);
bool ring_is_empty(const ring_t *r);
bool ring_is_full(const ring_t *r);
