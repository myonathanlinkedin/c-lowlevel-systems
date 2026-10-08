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

Graph* graph_create(size_t n);
void graph_free(Graph *g);
void graph_add_edge(Graph *g, size_t u, size_t v);
size_t graph_vertex_count(const Graph *g);
bool graph_is_simplicial(const Graph *g, size_t v);
bool kernelize(Graph *g, int k);   // returns true if reduced size ≤ k*k+2k

#ifdef __cplusplus
}
#endif
