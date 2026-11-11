#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdatomic.h>

typedef struct {
    void **buffer;          // storage for pointers
    size_t capacity;        // must be power of two
    size_t mask;            // capacity - 1, for fast modulo
    atomic_size_t head;     // index for consumer
    atomic_size_t tail;     // index for producer
} ring_t;
