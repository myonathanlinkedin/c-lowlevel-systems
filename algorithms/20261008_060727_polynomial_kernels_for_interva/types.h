#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    size_t n;               // original number of vertices
    uint8_t *adj;           // flattened adjacency matrix (n * n), 0/1 values
    bool *alive;            // vertex is still present
    size_t alive_cnt;       // count of alive vertices
} Graph;
