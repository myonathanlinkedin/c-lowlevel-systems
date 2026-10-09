#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#pragma once

#include "types.h"

/* Public API of the engine module. */
Graph *graph_create(size_t n);
void graph_destroy(Graph *g);
void graph_add_edge(Graph *g, size_t u, size_t v);
void graph_remove_edge(Graph *g, size_t u, size_t v);
bool graph_is_edge(const Graph *g, size_t u, size_t v);
bool graph_is_simplicial(const Graph *g, size_t v);
Graph *kernelize_interval_completion(Graph *g, size_t k);
