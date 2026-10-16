#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Helper to compute the index in the 1‑D adjacency array. */
static inline size_t idx(const Graph *g, size_t i, size_t j) {
    return i * g->n + j;
}

/* --------------------------------------------------------------------- */
Graph *graph_create(size_t n) {
    Graph *g = (Graph *)malloc(sizeof(Graph));
    if (!g) return NULL;
    g->n = n;
    if (n == 0) {
        g->adj = NULL;
        return g;
    }
    g->adj = (uint8_t *)calloc(n * n, sizeof(uint8_t));
    if (!g->adj) {
        free(g);
        return NULL;
    }
    return g;
}

/* --------------------------------------------------------------------- */
void graph_destroy(Graph *g) {
    if (!g) return;
    free(g->adj);
    free(g);
}

/* --------------------------------------------------------------------- */
void graph_add_edge(Graph *g, size_t u, size_t v) {
    assert(g != NULL);
    assert(u < g->n && v < g->n);
    assert(u != v);
    g->adj[idx(g, u, v)] = 1;
    g->adj[idx(g, v, u)] = 1;
}

/* --------------------------------------------------------------------- */
void graph_remove_edge(Graph *g, size_t u, size_t v) {
    assert(g != NULL);
    assert(u < g->n && v < g->n);
    assert(u != v);
    g->adj[idx(g, u, v)] = 0;
    g->adj[idx(g, v, u)] = 0;
}

/* --------------------------------------------------------------------- */
bool graph_is_edge(const Graph *g, size_t u, size_t v) {
    assert(g != NULL);
    if (u >= g->n || v >= g->n) return false;
    return g->adj[idx(g, u, v)] != 0;
}

/* --------------------------------------------------------------------- */
/* A vertex v is simplicial iff its neighbourhood (excluding v)
 * induces a complete subgraph.
 */
bool graph_is_simplicial(const Graph *g, size_t v) {
    assert(g != NULL);
    assert(v < g->n);
    /* Collect neighbours of v. */
    for (size_t i = 0; i < g->n; ++i) {
        if (i == v) continue;
        if (!graph_is_edge(g, v, i)) continue;
        /* For each neighbour i, ensure it is adjacent to every other neighbour j. */
        for (size_t j = i + 1; j < g->n; ++j) {
            if (j == v) continue;
            if (!graph_is_edge(g, v, j)) continue;
            if (!graph_is_edge(g, i, j))
                return false;
        }
    }
    return true;
}

/* --------------------------------------------------------------------- */
/* Internal helper: create a mapping from old vertex indices to new ones
 * after removal of a subset of vertices.
 */
static Graph *graph_subgraph(const Graph *orig, const bool *keep) {
    size_t new_n = 0;
    for (size_t i = 0; i < orig->n; ++i)
        if (keep[i]) ++new_n;

    Graph *sub = graph_create(new_n);
    if (!sub) return NULL;

    size_t *old_to_new = (size_t *)malloc(orig->n * sizeof(size_t));
    if (!old_to_new) {
        graph_destroy(sub);
        return NULL;
    }

    size_t cur = 0;
    for (size_t i = 0; i < orig->n; ++i) {
        old_to_new[i] = keep[i] ? cur++ : (size_t)-1;
    }

    for (size_t i = 0; i < orig->n; ++i) {
        if (!keep[i]) continue;
        for (size_t j = i + 1; j < orig->n; ++j) {
            if (!keep[j]) continue;
            if (graph_is_edge(orig, i, j))
                graph_add_edge(sub, old_to_new[i], old_to_new[j]);
        }
    }

    free(old_to_new);
    return sub;
}

/* --------------------------------------------------------------------- */
Graph *kernelize_interval_completion(Graph *g, size_t k) {
    assert(g != NULL);
    /* Repeatedly delete simplicial vertices. */
    bool changed = true;
    while (changed) {
        changed = false;
        bool *keep = (bool *)calloc(g->n, sizeof(bool));
        if (!keep) {
            /* Allocation failure – abort kernelization, return original graph. */
            return g;
        }
        for (size_t v = 0; v < g->n; ++v) {
            if (graph_is_simplicial(g, v)) {
                /* Vertex v can be safely removed. */
                keep[v] = false;
                changed = true;
            } else {
                keep[v] = true;
            }
        }
        if (changed) {
            Graph *reduced = graph_subgraph(g, keep);
            free(keep);
            graph_destroy(g);
            g = reduced;
            if (!g) return NULL; /* allocation failure */
        } else {
            free(keep);
        }
    }

    /* At this point no simplicial vertex remains.
     * The theoretical kernel bound for Interval Completion is
     * (k+1)^2 vertices.  We do not enforce the bound here; we only
     * document it for the caller.
     */
    (void)k; /* suppress unused‑parameter warning */
    return g;
}
