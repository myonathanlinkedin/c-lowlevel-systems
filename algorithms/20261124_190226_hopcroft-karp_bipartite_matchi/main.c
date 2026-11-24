#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <assert.h>

#include "types.h"
#include <stdio.h>

static void test_empty_graph(void) {
    HKGraph g;
    hk_init(&g, 0, 0);
    assert(hk_maximum_matching(&g) == 0);
    hk_destroy(&g);
}

static void test_single_edge(void) {
    HKGraph g;
    hk_init(&g, 1, 1);
    hk_add_edge(&g, 0, 0);
    assert(hk_maximum_matching(&g) == 1);
    hk_destroy(&g);
}

static void test_no_edges(void) {
    HKGraph g;
    hk_init(&g, 3, 3);
    assert(hk_maximum_matching(&g) == 0);
    hk_destroy(&g);
}

static void test_simple_triangle(void) {
    /* Left: 0,1   Right: 0,1
       Edges: 0-0, 0-1, 1-0
       Maximum matching size = 2 */
    HKGraph g;
    hk_init(&g, 2, 2);
    hk_add_edge(&g, 0, 0);
    hk_add_edge(&g, 0, 1);
    hk_add_edge(&g, 1, 0);
    assert(hk_maximum_matching(&g) == 2);
    hk_destroy(&g);
}

static void test_complete_bipartite(void) {
    const size_t N = 4;
    HKGraph g;
    hk_init(&g, N, N);
    for (size_t u = 0; u < N; ++u)
        for (size_t v = 0; v < N; ++v)
            hk_add_edge(&g, u, v);
    assert(hk_maximum_matching(&g) == N);
    hk_destroy(&g);
}

static void test_unequal_sides(void) {
    /* Left side 5, Right side 3, edges form a perfect matching of size 3 */
    HKGraph g;
    hk_init(&g, 5, 3);
    hk_add_edge(&g, 0, 0);
    hk_add_edge(&g, 1, 1);
    hk_add_edge(&g, 2, 2);
    hk_add_edge(&g, 3, 0);
    hk_add_edge(&g, 4, 1);
    assert(hk_maximum_matching(&g) == 3);
    hk_destroy(&g);
}

int main(void) {
    test_empty_graph();
    test_single_edge();
    test_no_edges();
    test_simple_triangle();
    test_complete_bipartite();
    test_unequal_sides();

    printf("All Hopcroft‑Karp unit tests passed.\n");
    return 0;
}
