#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#include "types.h"
#include <stdio.h>
#include <assert.h>

static void test_empty_graph(void)
{
    BipartiteGraph *g = bk_graph_create(0, 0);
    assert(g != NULL);
    HKState *st = hk_state_create(g);
    assert(st != NULL);
    int size = hopcroft_karp(g, st);
    assert(size == 0);
    hk_state_destroy(st);
    bk_graph_destroy(g);
}

static void test_single_edge(void)
{
    BipartiteGraph *g = bk_graph_create(1, 1);
    assert(g != NULL);
    assert(bk_graph_add_edge(g, 0, 0));
    HKState *st = hk_state_create(g);
    assert(st != NULL);
    int size = hopcroft_karp(g, st);
    assert(size == 1);
    assert(st->pair_u[0] == 0);
    assert(st->pair_v[0] == 0);
    hk_state_destroy(st);
    bk_graph_destroy(g);
}

static void test_multiple_edges(void)
{
    /* Graph:
     * U0 - V0, V1
     * U1 - V0
     * U2 - V1, V2
     * Expected maximum matching size = 3
     */
    BipartiteGraph *g = bk_graph_create(3, 3);
    assert(g != NULL);
    assert(bk_graph_add_edge(g, 0, 0));
    assert(bk_graph_add_edge(g, 0, 1));
    assert(bk_graph_add_edge(g, 1, 0));
    assert(bk_graph_add_edge(g, 2, 1));
    assert(bk_graph_add_edge(g, 2, 2));

    HKState *st = hk_state_create(g);
    assert(st != NULL);
    int size = hopcroft_karp(g, st);
    assert(size == 3);

    /* Verify that each vertex is matched at most once */
    int used_v[3] = {0};
    for (size_t u = 0; u < 3; ++u) {
        int v = st->pair_u[u];
        if (v != -1) {
            assert(v >= 0 && v < 3);
            assert(!used_v[v]);   /* each V appears at most once */
            used_v[v] = 1;
        }
    }

    hk_state_destroy(st);
    bk_graph_destroy(g);
}

static void test_disconnected_components(void)
{
    /* Two independent components:
     * Component A: U0-V0
     * Component B: U1-U2 (no edges) and V1-V2 (no edges)
     * Expected matching size = 1
     */
    BipartiteGraph *g = bk_graph_create(3, 3);
    assert(g != NULL);
    assert(bk_graph_add_edge(g, 0, 0));

    HKState *st = hk_state_create(g);
    assert(st != NULL);
    int size = hopcroft_karp(g, st);
    assert(size == 1);
    assert(st->pair_u[0] == 0);
    assert(st->pair_v[0] == 0);
    assert(st->pair_u[1] == -1);
    assert(st->pair_u[2] == -1);
    hk_state_destroy(st);
    bk_graph_destroy(g);
}

static void test_large_random(void)
{
    /* Simple deterministic "large" graph to avoid randomness.
     * Left side: 0..4, Right side: 0..4
     * Fully connected bipartite graph (complete).
     * Expected matching size = 5
     */
    const size_t N = 5;
    BipartiteGraph *g = bk_graph_create(N, N);
    assert(g != NULL);
    for (size_t u = 0; u < N; ++u) {
        for (size_t v = 0; v < N; ++v) {
            assert(bk_graph_add_edge(g, u, v));
        }
    }

    HKState *st = hk_state_create(g);
    assert(st != NULL);
    int size = hopcroft_karp(g, st);
    assert(size == (int)N);

    /* Ensure a perfect matching (bijection) */
    int used_v[5] = {0};
    for (size_t u = 0; u < N; ++u) {
        int v = st->pair_u[u];
        assert(v >= 0 && (size_t)v < N);
        assert(!used_v[v]);
        used_v[v] = 1;
    }

    hk_state_destroy(st);
    bk_graph_destroy(g);
}

int main(void)
{
    test_empty_graph();
    test_single_edge();
    test_multiple_edges();
    test_disconnected_components();
    test_large_random();

    printf("All Hopcroft‑Karp unit tests passed.\n");
    return 0;
}
