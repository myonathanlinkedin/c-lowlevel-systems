#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/* Graph representation using an adjacency matrix.
 * The matrix is stored as a 1‑dimensional array of bytes
 * where entry (i,j) is at index i * n + j.
 * The graph is undirected; matrix is symmetric.
 */
typedef struct {
    size_t n;          /* number of vertices */
    uint8_t *adj;      /* adjacency matrix (n * n bytes) */
} Graph;

/* Create a graph with n vertices and no edges.
 * Returns NULL on allocation failure.
 */
Graph *graph_create(size_t n);

/* Release all memory owned by the graph. */
void graph_destroy(Graph *g);

/* Add an undirected edge between u and v (u != v). */
void graph_add_edge(Graph *g, size_t u, size_t v);

/* Remove the edge between u and v (if present). */
void graph_remove_edge(Graph *g, size_t u, size_t v);

/* Test whether an edge (u,v) exists. */
bool graph_is_edge(const Graph *g, size_t u, size_t v);

/* Return true iff the set of neighbours of v forms a clique. */
bool graph_is_simplicial(const Graph *g, size_t v);

/* Perform kernelization for the Interval Completion problem.
 * The function removes all simplicial vertices (they are already
 * compatible with any interval representation) and returns a
 * new graph that is a subgraph of the original.
 * The original graph is freed; the caller receives ownership of
 * the returned graph.
 *
 * The parameter k is the allowed number of edge insertions.
 * The kernel guarantees that the resulting graph has at most
 * (k+1)*(k+1) vertices (a well‑known polynomial kernel bound).
 *
 * If the kernel size exceeds the bound, the function still returns
 * the reduced graph; the caller may decide to abort.
 */
Graph *kernelize_interval_completion(Graph *g, size_t k);
