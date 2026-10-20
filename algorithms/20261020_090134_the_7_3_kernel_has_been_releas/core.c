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

bool graph_add_edge(Graph *g, size_t u, size_t v) {
    if (!g || u >= g->n || v >= g->n) return false;
    g->adj[idx(g, u, v)] = 1;
    return true;
}

void graph_free(Graph *g) {
    if (!g) return;
    free(g->adj);
    free(g);
}

// Helper: test if subset (mask) is independent (no internal edges)
static bool is_independent(const Graph *g, uint32_t mask) {
    size_t n = g->n;
    for (size_t i = 0; i < n; ++i) {
        if (!(mask & (1u << i))) continue;
        for (size_t j = 0; j < n; ++j) {
            if (i == j) continue;
            if (mask & (1u << j)) {
                if (g->adj[idx(g, i, j)]) return false;
                if (g->adj[idx(g, j, i)]) return false;
            }
        }
    }
    return true;
}

// Helper: test if subset (mask) absorbs all vertices outside it
static bool is_absorbing(const Graph *g, uint32_t mask) {
    size_t n = g->n;
    for (size_t v = 0; v < n; ++v) {
        if (mask & (1u << v)) continue; // already in kernel
        bool has_edge = false;
        for (size_t u = 0; u < n; ++u) {
            if (mask & (1u << u)) {
                if (g->adj[idx(g, v, u)]) {
                    has_edge = true;
                    break;
                }
            }
        }
        if (!has_edge) return false;
    }
    return true;
}

bool compute_kernel(const Graph *g, uint32_t *out_mask) {
    if (!g || g->n > 32) return false; // limit to 32 vertices for bitmask simplicity
    size_t n = g->n;
    uint32_t total = (n == 32) ? 0xFFFFFFFFu : ((1u << n) - 1u);
    // Enumerate subsets in increasing size (optional but deterministic)
    for (uint32_t mask = 1; mask <= total; ++mask) {
        if (!is_independent(g, mask)) continue;
        if (!is_absorbing(g, mask)) continue;
        *out_mask = mask;
        return true;
    }
    return false; // no kernel
}
