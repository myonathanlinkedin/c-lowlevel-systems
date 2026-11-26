#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

static int edge_cmp(const void *a, const void *b) {
    const Edge *ea = (const Edge *)a;
    const Edge *eb = (const Edge *)b;
    if (ea->weight < eb->weight) return -1;
    if (ea->weight > eb->weight) return 1;
    return 0;
}

bool dsu_init(DSU *dsu, size_t n) {
    if (!dsu) return false;
    dsu->parent = (uint32_t *)malloc(n * sizeof(uint32_t));
    dsu->rank   = (uint32_t *)calloc(n, sizeof(uint32_t));
    if (!dsu->parent || !dsu->rank) {
        free(dsu->parent);
        free(dsu->rank);
        return false;
    }
    for (size_t i = 0; i < n; ++i) {
        dsu->parent[i] = (uint32_t)i;
    }
    dsu->size = n;
    return true;
}

void dsu_destroy(DSU *dsu) {
    if (!dsu) return;
    free(dsu->parent);
    free(dsu->rank);
    dsu->parent = NULL;
    dsu->rank = NULL;
    dsu->size = 0;
}

uint32_t dsu_find(DSU *dsu, uint32_t x) {
    assert(dsu && dsu->parent && x < dsu->size);
    if (dsu->parent[x] != x) {
        dsu->parent[x] = dsu_find(dsu, dsu->parent[x]); // path compression
    }
    return dsu->parent[x];
}

bool dsu_union(DSU *dsu, uint32_t x, uint32_t y) {
    uint32_t xr = dsu_find(dsu, x);
    uint32_t yr = dsu_find(dsu, y);
    if (xr == yr) return false;
    if (dsu->rank[xr] < dsu->rank[yr]) {
        dsu->parent[xr] = yr;
    } else if (dsu->rank[xr] > dsu->rank[yr]) {
        dsu->parent[yr] = xr;
    } else {
        dsu->parent[yr] = xr;
        dsu->rank[xr] += 1;
    }
    return true;
}

int32_t kruskal_mst(uint32_t n, const Edge *edges, size_t edge_cnt,
                   Edge *mst_edges, size_t *mst_cnt) {
    if (!edges || !mst_edges || !mst_cnt) return INT32_MAX;
    if (n == 0) {
        *mst_cnt = 0;
        return 0;
    }
    Edge *sorted = (Edge *)malloc(edge_cnt * sizeof(Edge));
    if (!sorted) return INT32_MAX;
    for (size_t i = 0; i < edge_cnt; ++i) sorted[i] = edges[i];
    qsort(sorted, edge_cnt, sizeof(Edge), edge_cmp);

    DSU dsu;
    if (!dsu_init(&dsu, n)) {
        free(sorted);
        return INT32_MAX;
    }

    size_t taken = 0;
    int32_t total = 0;
    for (size_t i = 0; i < edge_cnt && taken < n - 1; ++i) {
        const Edge *e = &sorted[i];
        if (e->u >= n || e->v >= n) continue; // ignore invalid vertices
        if (dsu_union(&dsu, e->u, e->v)) {
            mst_edges[taken++] = *e;
            total += e->weight;
        }
    }
    dsu_destroy(&dsu);
    free(sorted);
    *mst_cnt = taken;
    if (taken != n - 1) return INT32_MAX; // graph not connected
    return total;
}
