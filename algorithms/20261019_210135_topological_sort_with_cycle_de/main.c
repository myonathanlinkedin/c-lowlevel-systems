#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdio.h>
#include <assert.h>

static void test_acyclic(void) {
    Graph *g = graph_create(6);
    assert(g);
    // DAG:
    // 5 -> 2, 5 -> 0
    // 4 -> 0, 4 -> 1
    // 2 -> 3
    // 3 -> 1
    assert(graph_add_edge(g, 5, 2));
    assert(graph_add_edge(g, 5, 0));
    assert(graph_add_edge(g, 4, 0));
    assert(graph_add_edge(g, 4, 1));
    assert(graph_add_edge(g, 2, 3));
    assert(graph_add_edge(g, 3, 1));

    size_t order[6];
    size_t len = 0;
    bool has_cycle = false;
    assert(topological_sort(g, order, &len, &has_cycle));
    assert(!has_cycle);
    assert(len == 6);

    // Verify ordering respects edges
    // Helper lambda replaced by manual checks
    for (size_t i = 0; i < len; ++i) {
        for (size_t j = i + 1; j < len; ++j) {
            // if edge order[j] -> order[i] exists, it's invalid
            size_t src = order[j];
            size_t dst = order[i];
            // scan adjacency of src
            for (size_t k = 0; k < g->adj_sizes[src]; ++k) {
                if (g->adj[src][k] == dst) {
                    fprintf(stderr, "Invalid topological order: %zu before %zu\n", src, dst);
                    assert(0);
                }
            }
        }
    }

    graph_free(g);
    printf("test_acyclic passed\n");
}

static void test_cycle(void) {
    Graph *g = graph_create(3);
    assert(g);
    // Cycle: 0 -> 1 -> 2 -> 0
    assert(graph_add_edge(g, 0, 1));
    assert(graph_add_edge(g, 1, 2));
    assert(graph_add_edge(g, 2, 0));

    size_t order[3];
    size_t len = 0;
    bool has_cycle = false;
    assert(topological_sort(g, order, &len, &has_cycle));
    assert(has_cycle);
    assert(len < 3); // at least one vertex cannot be placed

    graph_free(g);
    printf("test_cycle passed\n");
}

static void test_self_loop(void) {
    Graph *g = graph_create(2);
    assert(g);
    assert(graph_add_edge(g, 0, 0)); // self‑loop creates a cycle
    size_t order[2];
    size_t len = 0;
    bool has_cycle = false;
    assert(topological_sort(g, order, &len, &has_cycle));
    assert(has_cycle);
    graph_free(g);
    printf("test_self_loop passed\n");
}

static void test_empty(void) {
    Graph *g = graph_create(0);
    assert(g);
    size_t order[1]; // unused
    size_t len = 0;
    bool has_cycle = false;
    assert(topological_sort(g, order, &len, &has_cycle));
    assert(!has_cycle);
    assert(len == 0);
    graph_free(g);
    printf("test_empty passed\n");
}

static void test_single_node(void) {
    Graph *g = graph_create(1);
    assert(g);
    size_t order[1];
    size_t len = 0;
    bool has_cycle = false;
    assert(topological_sort(g, order, &len, &has_cycle));
    assert(!has_cycle);
    assert(len == 1);
    assert(order[0] == 0);
    graph_free(g);
    printf("test_single_node passed\n");
}

int main(void) {
    test_acyclic();
    test_cycle();
    test_self_loop();
    test_empty();
    test_single_node();
    printf("All tests passed.\n");
    return 0;
}
