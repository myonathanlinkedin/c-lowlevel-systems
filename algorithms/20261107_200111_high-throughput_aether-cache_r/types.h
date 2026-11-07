#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdatomic.h>

typedef struct {
    size_t capacity;      // must be power of two
    size_t mask;          // capacity - 1
    void **buffer;        // array of pointers
    atomic_size_t head;   // write index
    atomic_size_t tail;   // read index
} ring_t;
