#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include "types.h"
#include <stdio.h>
#include <assert.h>

// Utility: print a mask as a set of vertex indices
static void print_mask(uint32_t mask, size_t n) {
    printf("{");
    bool first = true;
    for (size_t i = 0; i < n; ++i) {
        if (mask & (1u << i)) {
            if (!first) printf(", ");
            printf("%zu", i);
            first = false;
        }
    }
    printf("}");
}

// Unit test 1: empty graph (n=0) – kernel is empty set (by definition we treat as no kernel)
static void test_empty(void) {
    Graph *g = graph_create(0);
    assert(g != NULL);
    uint32_t mask = 0;
    bool ok = compute_kernel(g, &mask);
    assert(!ok); // no kernel for empty graph under our implementation
    graph_free(g);
}

// Unit test 2: single vertex, no edges – kernel is {0}
static void test_single_vertex(void) {
    Graph *g = graph_create(1);
    assert(g != NULL);
    uint32_t mask = 0;
    bool ok = compute_kernel(g, &mask);
    assert(ok);
    assert(mask == 1u);
    graph_free(g);
}

// Unit test 3: two vertices, edge 0->1 – kernel is {1}
static void test_two_vertices_one_edge(void) {
    Graph *g = graph_create(2);
    assert(g != NULL);
    assert(graph_add_edge(g, 0, 1));
    uint32_t mask = 0;
    bool ok = compute_kernel(g, &mask);
    assert(ok);
    assert(mask == (1u << 1));
    graph_free(g);
}

// Unit test 4: two vertices, edges both ways – no kernel (cannot be independent)
static void test_two_vertices_mutual(void) {
    Graph *g = graph_create(2);
    assert(g != NULL);
    assert(graph_add_edge(g, 0, 1));
    assert(graph_add_edge(g, 1, 0));
    uint32_t mask = 0;
    bool ok = compute_kernel(g, &mask);
    assert(!ok);
    graph_free(g);
}

// Unit test 5: three vertices forming a directed cycle 0->1->2->0 – kernel does not exist
static void test_three_cycle(void) {
    Graph *g = graph_create(3);
    assert(g != NULL);
    assert(graph_add_edge(g, 0, 1));
    assert(graph_add_edge(g, 1, 2));
    assert(graph_add_edge(g, 2, 0));
    uint32_t mask = 0;
    bool ok = compute_kernel(g, &mask);
    assert(!ok);
    graph_free(g);
}

// Unit test 6: three vertices, edges 0->1, 0->2 – kernel is {1,2}
static void test_three_star(void) {
    Graph *g = graph_create(3);
    assert(g != NULL);
    assert(graph_add_edge(g, 0, 1));
    assert(graph_add_edge(g, 0, 2));
    uint32_t mask = 0;
    bool ok = compute_kernel(g, &mask);
    assert(ok);
    assert(mask == ((1u << 1) | (1u << 2)));
    graph_free(g);
}

// Run all tests and report
int main(void) {
    printf("Running kernel computation unit tests...\n");
    test_empty();
    test_single_vertex();
    test_two_vertices_one_edge();
    test_two_vertices_mutual();
    test_three_cycle();
    test_three_star();
    printf("All tests passed.\n");
    return 0;
}
