#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>

static inline size_t idx(const Graph *g, size_t i, size_t j) {
    return i * g->n + j;
}

Graph* graph_create(size_t n) {
    Graph *g = (Graph*)malloc(sizeof(Graph));
    if (!g) return NULL;
    g->n = n;
    g->adj = (uint8_t*)calloc(n * n, sizeof(uint8_t));
    g->alive = (bool*)malloc(n * sizeof(bool));
    if (!g->adj || !g->alive) {
        free(g->adj);
        free(g->alive);
        free(g);
        return NULL;
    }
    memset(g->alive, 1, n * sizeof(bool));
    g->alive_cnt = n;
    return g;
}

void graph_free(Graph *g) {
    if (!g) return;
    free(g->adj);
    free(g->alive);
    free(g);
}

void graph_add_edge(Graph *g, size_t u, size_t v) {
    if (u >= g->n || v >= g->n || u == v) return;
    g->adj[idx(g, u, v)] = 1;
    g->adj[idx(g, v, u)] = 1;
}

size_t graph_vertex_count(const Graph *g) {
    return g->alive_cnt;
}

/* Returns true if the neighbors of v (among alive vertices) form a clique. */
bool graph_is_simplicial(const Graph *g, size_t v) {
    if (!g->alive[v]) return false;
    // collect neighbors
    for (size_t i = 0; i < g->n; ++i) {
        if (!g->alive[i] || i == v) continue;
        if (g->adj[idx(g, v, i)] == 0) continue; // not a neighbor
        // check that i is adjacent to every other neighbor of v
        for (size_t j = i + 1; j < g->n; ++j) {
            if (!g->alive[j] || j == v) continue;
            if (g->adj[idx(g, v, j)] == 0) continue;
            if (g->adj[idx(g, i, j)] == 0) return false;
        }
    }
    return true;
}

/* Remove vertex v from the graph (mark dead). */
static void remove_vertex(Graph *g, size_t v) {
    if (!g->alive[v]) return;
    g->alive[v] = false;
    g->alive_cnt--;
}

/* Simple kernelization for Interval Completion:
   - Repeatedly delete simplicial vertices (they never need fill edges).
   - If after reductions the vertex count exceeds k*k + 2k, we deem the kernel too large.
   Returns true if the reduced instance respects the size bound.
*/
bool kernelize(Graph *g, int k) {
    if (k < 0) return false;
    bool changed = true;
    while (changed) {
        changed = false;
        for (size_t v = 0; v < g->n; ++v) {
            if (g->alive[v] && graph_is_simplicial(g, v)) {
                remove_vertex(g, v);
                changed = true;
            }
        }
    }
    size_t bound = (size_t)k * (size_t)k + (size_t)2 * (size_t)k;
    return g->alive_cnt <= bound;
}
