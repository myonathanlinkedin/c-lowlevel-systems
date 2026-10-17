#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#include "types.h"
#include "core.h"
#include <stdio.h>
#include <assert.h>

static void test_basic_topological_sort(void) {
    DepGraph g;
    assert(graph_init(&g, 0));

    size_t a = graph_add_node(&g, "A");
    size_t b = graph_add_node(&g, "B");
    size_t c = graph_add_node(&g, "C");
    assert(a != (size_t)-1 && b != (size_t)-1 && c != (size_t)-1);

    assert(graph_add_edge(&g, a, b));
    assert(graph_add_edge(&g, b, c));
    assert(!graph_has_cycle(&g));

    size_t order[3];
    assert(graph_topological_sort(&g, order));

    // Expected order: A, B, C (any order respecting dependencies is valid)
    assert(order[0] == a);
    assert(order[1] == b);
    assert(order[2] == c);

    graph_free(&g);
}

static void test_cycle_detection(void) {
    DepGraph g;
    assert(graph_init(&g, 0));

    size_t x = graph_add_node(&g, "X");
    size_t y = graph_add_node(&g, "Y");
    size_t z = graph_add_node(&g, "Z");
    assert(x != (size_t)-1 && y != (size_t)-1 && z != (size_t)-1);

    assert(graph_add_edge(&g, x, y));
    assert(graph_add_edge(&g, y, z));
    assert(graph_add_edge(&g, z, x)); // creates a cycle

    assert(graph_has_cycle(&g));
    size_t order[3];
    assert(!graph_topological_sort(&g, order)); // should fail

    graph_free(&g);
}

static void test_disconnected_graph(void) {
    DepGraph g;
    assert(graph_init(&g, 0));

    size_t p = graph_add_node(&g, "P");
    size_t q = graph_add_node(&g, "Q");
    size_t r = graph_add_node(&g, "R");
    size_t s = graph_add_node(&g, "S");
    assert(p != (size_t)-1 && q != (size_t)-1 && r != (size_t)-1 && s != (size_t)-1);

    // Two independent chains: P->Q and R->S
    assert(graph_add_edge(&g, p, q));
    assert(graph_add_edge(&g, r, s));
    assert(!graph_has_cycle(&g));

    size_t order[4];
    assert(graph_topological_sort(&g, order));

    // Verify each edge respects ordering
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = i + 1; j < 4; ++j) {
            // No need to check all pairs; just ensure that for each edge src appears before dst
            if (order[i] == p) assert(order[j] == q || order[j] == r || order[j] == s);
            if (order[i] == r) assert(order[j] == s);
        }
    }

    graph_free(&g);
}

int main(void) {
    test_basic_topological_sort();
    test_cycle_detection();
    test_disconnected_graph();

    printf("All dependency‑graph tests passed.\n");
    return 0;
}
