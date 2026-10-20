#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    size_t n;          // number of vertices
    uint8_t *adj;      // adjacency matrix (row-major, 0/1 values)
} Graph;
