
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    char *name;          // dynamically allocated name string
    size_t *edges;       // array of target node indices
    size_t edge_count;   // number of outgoing edges
    size_t edge_cap;     // capacity of edges array
} GraphNode;

typedef struct {
    GraphNode *nodes;    // array of nodes
    size_t node_count;   // current number of nodes
    size_t node_cap;     // capacity of nodes array
} DepGraph;
