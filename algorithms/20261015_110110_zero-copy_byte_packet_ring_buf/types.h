#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t *data;
    size_t   len;
} packet_t;

typedef struct {
    packet_t *buf;      // array of packets
    size_t    capacity; // power‑of‑two size
    size_t    mask;     // capacity‑1 for fast modulo
    size_t    head;     // index of next pop
    size_t    tail;     // index of next push
} ring_buffer_t;
