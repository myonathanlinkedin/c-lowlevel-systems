#include "types.h"
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static bool ensure_node_capacity(DepGraph *g, size_t needed) {
    if (needed <= g->node_cap) return true;
    size_t new_cap = g->node_cap ? g->node_cap * 2 : 4;
    while (new_cap < needed) new_cap *= 2;
    GraphNode *tmp = (GraphNode *)realloc(g->nodes, new_cap * sizeof(GraphNode));
    if (!tmp) return false;
    g->nodes = tmp;
    g->node_cap = new_cap;
    return true;
}

static bool ensure_edge_capacity(GraphNode *n, size_t needed) {
    if (needed <= n->edge_cap) return true;
    size_t new_cap = n->edge_cap ? n->edge_cap * 2 : 4;
    while (new_cap < needed) new_cap *= 2;
    size_t *tmp = (size_t *)realloc(n->edges, new_cap * sizeof(size_t));
    if (!tmp) return false;
    n->edges = tmp;
    n->edge_cap = new_cap;
    return true;
}

bool graph_init(DepGraph *g, size_t initial_capacity) {
    if (!g) return false;
    g->nodes = NULL;
    g->node_count = 0;
    g->node_cap = 0;
    if (initial_capacity == 0) initial_capacity = 4;
    return ensure_node_capacity(g, initial_capacity);
}

void graph_free(DepGraph *g) {
    if (!g) return;
    for (size_t i = 0; i < g->node_count; ++i) {
        free(g->nodes[i].name);
        free(g->nodes[i].edges);
    }
    free(g->nodes);
    g->nodes = NULL;
    g->node_count = g->node_cap = 0;
}

size_t graph_add_node(DepGraph *g, const char *name) {
    if (!g || !name) return (size_t)-1;
    if (!ensure_node_capacity(g, g->node_count + 1)) return (size_t)-1;
    GraphNode *n = &g->nodes[g->node_count];
    n->name = strdup(name);
    if (!n->name) return (size_t)-1;
    n->edges = NULL;
    n->edge_count = 0;
    n->edge_cap = 0;
    return g->node_count++;
}

bool graph_add_edge(DepGraph *g, size_t src, size_t dst) {
    if (!g) return false;
    if (src >= g->node_count || dst >= g->node_count) return false;
    GraphNode *s = &g->nodes[src];
    if (!ensure_edge_capacity(s, s->edge_count + 1)) return false;
    s->edges[s->edge_count++] = dst;
    return true;
}

/* Cycle detection using DFS with three‑state marking:
   0 = unvisited, 1 = visiting, 2 = visited */
static bool dfs_cycle(const DepGraph *g, size_t v, unsigned char *state) {
    state[v] = 1; // visiting
    const GraphNode *n = &g->nodes[v];
    for (size_t i = 0; i < n->edge_count; ++i) {
        size_t w = n->edges[i];
        if (state[w] == 1) return true;          // back edge -> cycle
        if (state[w] == 0 && dfs_cycle(g, w, state)) return true;
    }
    state[v] = 2; // visited
    return false;
}

bool graph_has_cycle(const DepGraph *g) {
    if (!g) return false;
    unsigned char *state = (unsigned char *)calloc(g->node_count, sizeof(unsigned char));
    if (!state) return false; // treat allocation failure as no cycle (conservative)
    bool has = false;
    for (size_t i = 0; i < g->node_count && !has; ++i) {
        if (state[i] == 0 && dfs_cycle(g, i, state)) has = true;
    }
    free(state);
    return has;
}

/* Kahn's algorithm for topological sorting */
bool graph_topological_sort(const DepGraph *g, size_t *order_out) {
    if (!g || !order_out) return false;
    size_t n = g->node_count;
    size_t *in_deg = (size_t *)calloc(n, sizeof(size_t));
    if (!in_deg) return false;

    // Compute in‑degrees
    for (size_t i = 0; i < n; ++i) {
        const GraphNode *node = &g->nodes[i];
        for (size_t e = 0; e < node->edge_count; ++e) {
            ++in_deg[node->edges[e]];
        }
    }

    // Simple queue implemented as circular buffer
    size_t *queue = (size_t *)malloc(n * sizeof(size_t));
    if (!queue) { free(in_deg); return false; }
    size_t head = 0, tail = 0;

    for (size_t i = 0; i < n; ++i) {
        if (in_deg[i] == 0) queue[tail++] = i;
    }

    size_t idx = 0;
    while (head < tail) {
        size_t v = queue[head++];
        order_out[idx++] = v;
        const GraphNode *node = &g->nodes[v];
        for (size_t e = 0; e < node->edge_count; ++e) {
            size_t w = node->edges[e];
            if (--in_deg[w] == 0) queue[tail++] = w;
        }
    }

    free(queue);
    free(in_deg);
    return idx == n; // true iff graph was acyclic
}
