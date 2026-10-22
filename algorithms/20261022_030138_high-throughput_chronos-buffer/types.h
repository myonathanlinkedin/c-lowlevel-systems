
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdatomic.h>

typedef struct {
    size_t capacity;          // total slots, must be power of two
    size_t mask;              // capacity - 1, for fast modulo
    _Atomic size_t head;      // index for consumer
    _Atomic size_t tail;      // index for producer
    void **buffer;            // array of void* elements
} ChronosRingBuffer;
