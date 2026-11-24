#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

/* Simple dynamic array used for adjacency lists */
typedef struct {
    size_t *data;
    size_t size;
    size_t capacity;
} DynArray;

/* Core structure representing a bipartite graph for Hopcroft‑Karp */
typedef struct {
    size_t n_left;      /* number of vertices on left side (U) */
    size_t n_right;     /* number of vertices on right side (V) */
    DynArray *adj;      /* adjacency list for each left vertex (size n_left) */
    int *pair_u;        /* matched right vertex for each left vertex, -1 if free */
    int *pair_v;        /* matched left vertex for each right vertex, -1 if free */
    int *dist;          /* distance labels used by BFS */
} HKGraph;

/* Public API */
void hk_init(HKGraph *g, size_t n_left, size_t n_right);
void hk_destroy(HKGraph *g);
void hk_add_edge(HKGraph *g, size_t u_left, size_t v_right);
size_t hk_maximum_matching(HKGraph *g);

/* Helper functions for DynArray (internal use, but exposed for testing) */
void dynarray_init(DynArray *a);
void dynarray_push(DynArray *a, size_t value);
void dynarray_free(DynArray *a);
