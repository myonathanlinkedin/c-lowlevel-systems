
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stddef.h>

#define RING_CAPACITY 1024

typedef struct {
    atomic_uint head;
    atomic_uint tail;
    uint64_t data[RING_CAPACITY];
} RingBuffer;
