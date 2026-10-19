#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include <stddef.h>
#include <stdbool.h>

typedef struct Graph {
    size_t V;               // number of vertices
    size_t *adj_sizes;      // number of outgoing edges per vertex
    size_t **adj;           // adjacency lists (array of arrays)
} Graph;
