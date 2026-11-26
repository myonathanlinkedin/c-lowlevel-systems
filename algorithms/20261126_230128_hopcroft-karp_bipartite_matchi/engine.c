#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*--------------------------------------------------------------
 * Internal helpers
 *--------------------------------------------------------------*/

static bool bfs(const BipartiteGraph *g, HKState *st)
{
    const size_t INF = (size_t)-1;
    size_t q_front = 0, q_back = 0;
    size_t *queue = (size_t *)malloc(g->n_left * sizeof(size_t));
    if (!queue) return false;   /* allocation failure */

    for (size_t u = 0; u < g->n_left; ++u) {
        if (st->pair_u[u] == -1) {
            st->dist[u] = 0;
            queue[q_back++] = u;
        } else {
            st->dist[u] = INF;
        }
    }

    size_t dist_nil = INF;

    while (q_front < q_back) {
        size_t u = queue[q_front++];
        if (st->dist[u] < dist_nil) {
            for (size_t i = 0; i < g->adj_len[u]; ++i) {
                int v = g->adj[u][i];
                int pu = st->pair_v[v];
                if (pu == -1) {
                    dist_nil = st->dist[u] + 1;
                } else if (st->dist[pu] == INF) {
                    st->dist[pu] = st->dist[u] + 1;
                    queue[q_back++] = (size_t)pu;
                }
            }
        }
    }

    free(queue);
    return dist_nil != INF;
}

static bool dfs(const BipartiteGraph *g, HKState *st, size_t u)
{
    for (size_t i = 0; i < g->adj_len[u]; ++i) {
        int v = g->adj[u][i];
        int pu = st->pair_v[v];
        if (pu == -1 ||
            (st->dist[pu] == st->dist[u] + 1 && dfs(g, st, (size_t)pu))) {
            st->pair_u[u] = v;
            st->pair_v[v] = (int)u;
            return true;
        }
    }
    st->dist[u] = (size_t)-1;   /* mark as dead end */
    return false;
}

/*--------------------------------------------------------------
 * Public API implementation
 *--------------------------------------------------------------*/

BipartiteGraph *bk_graph_create(size_t n_left, size_t n_right)
{
    BipartiteGraph *g = (BipartiteGraph *)malloc(sizeof(BipartiteGraph));
    if (!g) return NULL;

    g->n_left = n_left;
    g->n_right = n_right;

    g->adj = (int **)calloc(n_left, sizeof(int *));
    g->adj_len = (size_t *)calloc(n_left, sizeof(size_t));
    if (!g->adj || !g->adj_len) {
        bk_graph_destroy(g);
        return NULL;
    }
    return g;
}

void bk_graph_destroy(BipartiteGraph *g)
{
    if (!g) return;
    if (g->adj) {
        for (size_t i = 0; i < g->n_left; ++i) {
            free(g->adj[i]);
        }
        free(g->adj);
    }
    free(g->adj_len);
    free(g);
}

bool bk_graph_add_edge(BipartiteGraph *g, size_t u, size_t v)
{
    if (!g || u >= g->n_left || v >= g->n_right) return false;

    size_t old_len = g->adj_len[u];
    int *new_list = (int *)realloc(g->adj[u], (old_len + 1) * sizeof(int));
    if (!new_list) return false;

    new_list[old_len] = (int)v;
    g->adj[u] = new_list;
    g->adj_len[u] = old_len + 1;
    return true;
}

HKState *hk_state_create(const BipartiteGraph *g)
{
    HKState *st = (HKState *)malloc(sizeof(HKState));
    if (!st) return NULL;

    st->n_left = g->n_left;
    st->n_right = g->n_right;

    st->pair_u = (int *)malloc(g->n_left * sizeof(int));
    st->pair_v = (int *)malloc(g->n_right * sizeof(int));
    st->dist   = (int *)malloc(g->n_left * sizeof(int));

    if (!st->pair_u || !st->pair_v || !st->dist) {
        hk_state_destroy(st);
        return NULL;
    }

    for (size_t i = 0; i < g->n_left; ++i)  st->pair_u[i] = -1;
    for (size_t i = 0; i < g->n_right; ++i) st->pair_v[i] = -1;
    for (size_t i = 0; i < g->n_left; ++i)  st->dist[i]   = 0;

    return st;
}

void hk_state_destroy(HKState *st)
{
    if (!st) return;
    free(st->pair_u);
    free(st->pair_v);
    free(st->dist);
    free(st);
}

int hopcroft_karp(const BipartiteGraph *g, HKState *st)
{
    assert(g && st);
    int matching = 0;

    while (bfs(g, st)) {
        for (size_t u = 0; u < g->n_left; ++u) {
            if (st->pair_u[u] == -1) {
                if (dfs(g, st, u))
                    ++matching;
            }
        }
    }
    return matching;
}
