#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

Graph* graph_create(size_t V) {
    Graph *g = (Graph *)malloc(sizeof(Graph));
    if (!g) return NULL;
    g->V = V;
    g->adj_sizes = (size_t *)calloc(V, sizeof(size_t));
    g->adj = (size_t **)calloc(V, sizeof(size_t *));
    if (!g->adj_sizes || !g->adj) {
        free(g->adj);
        free(g->adj_sizes);
        free(g);
        return NULL;
    }
    return g;
}

void graph_free(Graph *g) {
    if (!g) return;
    for (size_t i = 0; i < g->V; ++i) {
        free(g->adj[i]);
    }
    free(g->adj);
    free(g->adj_sizes);
    free(g);
}

bool graph_add_edge(Graph *g, size_t src, size_t dst) {
    if (!g || src >= g->V || dst >= g->V) return false;
    size_t cur = g->adj_sizes[src];
    size_t *newlist = (size_t *)realloc(g->adj[src], (cur + 1) * sizeof(size_t));
    if (!newlist) return false;
    newlist[cur] = dst;
    g->adj[src] = newlist;
    g->adj_sizes[src] = cur + 1;
    return true;
}

static bool compute_indegrees(const Graph *g, size_t *indeg) {
    // indeg must be zero‑initialised by caller
    for (size_t u = 0; u < g->V; ++u) {
        for (size_t i = 0; i < g->adj_sizes[u]; ++i) {
            size_t v = g->adj[u][i];
            ++indeg[v];
        }
    }
    return true;
}

bool topological_sort(const Graph *g, size_t *order, size_t *order_len, bool *has_cycle) {
    if (!g || !order || !order_len || !has_cycle) return false;
    *order_len = 0;
    *has_cycle = false;

    size_t *indeg = (size_t *)calloc(g->V, sizeof(size_t));
    if (!indeg) return false;

    compute_indegrees(g, indeg);

    // Simple queue implementation using a circular buffer
    size_t *queue = (size_t *)malloc(g->V * sizeof(size_t));
    if (!queue) {
        free(indeg);
        return false;
    }
    size_t head = 0, tail = 0, qsize = 0;

    for (size_t v = 0; v < g->V; ++v) {
        if (indeg[v] == 0) {
            queue[tail++] = v;
            ++qsize;
        }
    }

    while (qsize) {
        size_t u = queue[head++];
        --qsize;
        order[(*order_len)++] = u;

        for (size_t i = 0; i < g->adj_sizes[u]; ++i) {
            size_t v = g->adj[u][i];
            assert(indeg[v] > 0);
            if (--indeg[v] == 0) {
                queue[tail++] = v;
                ++qsize;
            }
        }
    }

    if (*order_len != g->V) {
        *has_cycle = true;
    }

    free(queue);
    free(indeg);
    return true;
}
