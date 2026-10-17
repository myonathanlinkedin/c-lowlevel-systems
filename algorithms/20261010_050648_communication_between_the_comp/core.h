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

// Initialise a dependency graph with an initial capacity.
// Returns true on success, false on allocation failure.
bool graph_init(DepGraph *g, size_t initial_capacity);

// Release all memory owned by the graph.
void graph_free(DepGraph *g);

// Add a node with the given name. Returns the node index (>=0) on success,
// or (size_t)-1 on failure.
size_t graph_add_node(DepGraph *g, const char *name);

// Add a directed edge from `src` to `dst`. Returns true on success.
bool graph_add_edge(DepGraph *g, size_t src, size_t dst);

// Detect if the current graph contains a cycle.
bool graph_has_cycle(const DepGraph *g);

// Compute a topological ordering of the nodes.
// `order_out` must point to an array of size at least g->node_count.
// Returns true if a valid ordering exists (i.e., graph is acyclic).
bool graph_topological_sort(const DepGraph *g, size_t *order_out);

#ifdef __cplusplus
}
#endif
