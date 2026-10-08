#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdio.h>
#include <assert.h>

static void test_simplicial_detection(void) {
    Graph *g = graph_create(4);
    // Complete graph K4 -> every vertex is simplicial
    graph_add_edge(g, 0, 1);
    graph_add_edge(g, 0, 2);
    graph_add_edge(g, 0, 3);
    graph_add_edge(g, 1, 2);
    graph_add_edge(g, 1, 3);
    graph_add_edge(g, 2, 3);
    for (size_t v = 0; v < 4; ++v) {
        assert(graph_is_simplicial(g, v));
    }
    graph_free(g);
}

static void test_non_simplicial(void) {
    Graph *g = graph_create(4);
    // Path 0-1-2-3
    graph_add_edge(g, 0, 1);
    graph_add_edge(g, 1, 2);
    graph_add_edge(g, 2, 3);
    assert(!graph_is_simplicial(g, 1));
    assert(!graph_is_simplicial(g, 2));
    assert(graph_is_simplicial(g, 0));
    assert(graph_is_simplicial(g, 3));
    graph_free(g);
}

static void test_kernelization_small_k(void) {
    Graph *g = graph_create(5);
    // Star centered at 0 (0 connected to all others)
    for (size_t i = 1; i < 5; ++i) graph_add_edge(g, 0, i);
    // Leaves are simplicial, center is not.
    bool ok = kernelize(g, 1); // bound = 1*1+2*1 = 3
    // After removing leaves, only vertex 0 remains => size 1 ≤ 3
    assert(ok);
    assert(graph_vertex_count(g) == 1);
    graph_free(g);
}

static void test_kernelization_exceeds_bound(void) {
    Graph *g = graph_create(6);
    // Cycle C6 (no simplicial vertices)
    for (size_t i = 0; i < 6; ++i) {
        graph_add_edge(g, i, (i + 1) % 6);
    }
    bool ok = kernelize(g, 1); // bound = 3, but we have 6 vertices
    assert(!ok);
    assert(graph_vertex_count(g) == 6);
    graph_free(g);
}

int main(void) {
    test_simplicial_detection();
    test_non_simplicial();
    test_kernelization_small_k();
    test_kernelization_exceeds_bound();
    printf("All tests passed.\n");
    return 0;
}
