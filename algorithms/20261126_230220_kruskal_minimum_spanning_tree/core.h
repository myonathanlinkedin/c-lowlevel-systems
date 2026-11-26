#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#pragma once
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

bool dsu_init(DSU *dsu, size_t n);
void dsu_destroy(DSU *dsu);
uint32_t dsu_find(DSU *dsu, uint32_t x);
bool dsu_union(DSU *dsu, uint32_t x, uint32_t y);

/* 
 * kruskal_mst:
 *   n          - number of vertices (0..n-1)
 *   edges      - input edge array (unsorted)
 *   edge_cnt   - number of edges in input array
 *   mst_edges  - output buffer for MST edges (must have capacity >= n-1)
 *   mst_cnt    - output pointer for number of edges placed in mst_edges
 * Returns total weight of MST, or INT32_MAX if MST cannot be formed.
 */
int32_t kruskal_mst(uint32_t n, const Edge *edges, size_t edge_cnt,
                   Edge *mst_edges, size_t *mst_cnt);

#ifdef __cplusplus
}
#endif
