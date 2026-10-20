#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#pragma once
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Create a directed graph with n vertices (0-indexed). Returns NULL on allocation failure.
Graph* graph_create(size_t n);

// Add a directed edge u -> v. Returns false if indices out of range.
bool graph_add_edge(Graph *g, size_t u, size_t v);

// Free graph resources.
void graph_free(Graph *g);

// Compute a kernel of the directed graph.
// If a kernel exists, returns true and stores its vertex set as a bitmask in *out_mask (bit i set => vertex i in kernel).
// If no kernel exists, returns false and *out_mask is undefined.
bool compute_kernel(const Graph *g, uint32_t *out_mask);

#ifdef __cplusplus
}
#endif
