
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/*--------------------------------------------------------------
 * Data structures for Hopcroft‑Karp bipartite matching
 *--------------------------------------------------------------*/

typedef struct {
    size_t n_left;      /* number of vertices on left side (U) */
    size_t n_right;     /* number of vertices on right side (V) */
    int **adj;          /* adjacency list for each left vertex */
    size_t *adj_len;    /* length of each adjacency list */
} BipartiteGraph;

/* Matching result */
typedef struct {
    int *pair_u;        /* pair_u[u] = matched v or -1 (NIL) */
    int *pair_v;        /* pair_v[v] = matched u or -1 (NIL) */
    int *dist;          /* distance labels used by BFS */
    size_t n_left;
    size_t n_right;
} HKState;

/*--------------------------------------------------------------
 * Public API
 *--------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/* Allocate and initialise a bipartite graph with given sizes.
 * Returns NULL on allocation failure. */
BipartiteGraph *bk_graph_create(size_t n_left, size_t n_right);

/* Free all memory associated with a graph. */
void bk_graph_destroy(BipartiteGraph *g);

/* Add an undirected edge (u in U, v in V).  Returns true on success. */
bool bk_graph_add_edge(BipartiteGraph *g, size_t u, size_t v);

/* Initialise the Hopcroft‑Karp state for a given graph.
 * Returns NULL on allocation failure. */
HKState *hk_state_create(const BipartiteGraph *g);

/* Release the Hopcroft‑Karp state. */
void hk_state_destroy(HKState *st);

/* Compute maximum matching size.  Returns the cardinality of the
 * maximum matching.  The matching itself is stored in st->pair_u /
 * st->pair_v (value -1 denotes unmatched). */
int hopcroft_karp(const BipartiteGraph *g, HKState *st);

#ifdef __cplusplus
}
#endif
