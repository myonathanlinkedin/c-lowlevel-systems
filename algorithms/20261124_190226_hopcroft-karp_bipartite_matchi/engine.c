#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include "types.h"
#include <string.h>
#include <stdio.h>

/* ---------- DynArray implementation ---------- */
static void dynarray_ensure_capacity(DynArray *a, size_t min_cap) {
    if (a->capacity >= min_cap) return;
    size_t new_cap = a->capacity ? a->capacity * 2 : 4;
    while (new_cap < min_cap) new_cap *= 2;
    size_t *new_data = (size_t *)realloc(a->data, new_cap * sizeof(size_t));
    assert(new_data != NULL);
    a->data = new_data;
    a->capacity = new_cap;
}

void dynarray_init(DynArray *a) {
    a->data = NULL;
    a->size = 0;
    a->capacity = 0;
}

void dynarray_push(DynArray *a, size_t value) {
    dynarray_ensure_capacity(a, a->size + 1);
    a->data[a->size++] = value;
}

void dynarray_free(DynArray *a) {
    free(a->data);
    a->data = NULL;
    a->size = a->capacity = 0;
}

/* ---------- HKGraph implementation ---------- */
static void bfs(const HKGraph *g, int NIL) {
    const size_t INF = (size_t)-1;
    for (size_t u = 0; u < g->n_left; ++u) {
        if (g->pair_u[u] == NIL) {
            g->dist[u] = 0;
        } else {
            g->dist[u] = (int)INF;
        }
    }

    int *queue = (int *)malloc(g->n_left * sizeof(int));
    assert(queue != NULL);
    size_t qhead = 0, qtail = 0;

    for (size_t u = 0; u < g->n_left; ++u) {
        if (g->pair_u[u] == NIL) {
            queue[qtail++] = (int)u;
        }
    }

    int dist_nil = (int)INF;

    while (qhead < qtail) {
        int u = queue[qhead++];
        if ((size_t)g->dist[u] < (size_t)dist_nil) {
            const DynArray *adj_u = &g->adj[u];
            for (size_t i = 0; i < adj_u->size; ++i) {
                int v = (int)adj_u->data[i];
                int pu = g->pair_v[v];
                if (pu == NIL) {
                    dist_nil = g->dist[u] + 1;
                } else if (g->dist[pu] == (int)INF) {
                    g->dist[pu] = g->dist[u] + 1;
                    queue[qtail++] = pu;
                }
            }
        }
    }

    free(queue);
}

/* DFS returns true if an augmenting path starting from u is found */
static bool dfs(HKGraph *g, int u, int NIL) {
    const DynArray *adj_u = &g->adj[u];
    for (size_t i = 0; i < adj_u->size; ++i) {
        int v = (int)adj_u->data[i];
        int pu = g->pair_v[v];
        if (pu == NIL || (g->dist[pu] == g->dist[u] + 1 && dfs(g, pu, NIL))) {
            g->pair_u[u] = v;
            g->pair_v[v] = u;
            return true;
        }
    }
    g->dist[u] = -1; /* mark as dead end */
    return false;
}

/* Public API */
void hk_init(HKGraph *g, size_t n_left, size_t n_right) {
    g->n_left = n_left;
    g->n_right = n_right;
    g->adj = (DynArray *)calloc(n_left, sizeof(DynArray));
    assert(g->adj != NULL);
    for (size_t i = 0; i < n_left; ++i) {
        dynarray_init(&g->adj[i]);
    }
    g->pair_u = (int *)malloc(n_left * sizeof(int));
    g->pair_v = (int *)malloc(n_right * sizeof(int));
    g->dist   = (int *)malloc(n_left * sizeof(int));
    assert(g->pair_u && g->pair_v && g->dist);
    for (size_t i = 0; i < n_left; ++i) g->pair_u[i] = -1;
    for (size_t i = 0; i < n_right; ++i) g->pair_v[i] = -1;
}

void hk_destroy(HKGraph *g) {
    for (size_t i = 0; i < g->n_left; ++i) {
        dynarray_free(&g->adj[i]);
    }
    free(g->adj);
    free(g->pair_u);
    free(g->pair_v);
    free(g->dist);
    g->adj = NULL;
    g->pair_u = g->pair_v = g->dist = NULL;
    g->n_left = g->n_right = 0;
}

void hk_add_edge(HKGraph *g, size_t u_left, size_t v_right) {
    assert(u_left < g->n_left);
    assert(v_right < g->n_right);
    dynarray_push(&g->adj[u_left], v_right);
}

/* Returns size of maximum matching */
size_t hk_maximum_matching(HKGraph *g) {
    const int NIL = -1;
    size_t matching = 0;

    while (true) {
        bfs(g, NIL);
        bool progress = false;
        for (size_t u = 0; u < g->n_left; ++u) {
            if (g->pair_u[u] == NIL) {
                if (dfs(g, (int)u, NIL)) {
                    ++matching;
                    progress = true;
                }
            }
        }
        if (!progress) break;
    }
    return matching;
}
