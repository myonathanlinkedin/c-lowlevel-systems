#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include <stddef.h>
#include <stdatomic.h>
#include <stdbool.h>

typedef struct {
    size_t capacity;          // power‑of‑two size of the buffer
    size_t mask;              // capacity‑1, for fast modulo
    atomic_size_t head;       // write index (producer)
    atomic_size_t tail;       // read index  (consumer)
    int *buffer;              // storage for elements
} ring_buffer_t;
