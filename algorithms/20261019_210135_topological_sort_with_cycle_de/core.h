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

// Create a graph with V vertices (0..V-1). Returns NULL on allocation failure.
Graph* graph_create(size_t V);

// Release all memory owned by the graph.
void graph_free(Graph *g);

// Add a directed edge src -> dst. Returns false on allocation failure or invalid indices.
bool graph_add_edge(Graph *g, size_t src, size_t dst);

// Perform topological sort using Kahn's algorithm.
//   order      : caller‑allocated array of size g->V to receive the sorted vertices.
//   order_len  : receives the number of vertices placed into order (0 on failure).
//   has_cycle  : receives true if a cycle exists, false otherwise.
// Returns true on successful execution (allocation succeeded), false on internal error.
bool topological_sort(const Graph *g, size_t *order, size_t *order_len, bool *has_cycle);

#ifdef __cplusplus
}
#endif
