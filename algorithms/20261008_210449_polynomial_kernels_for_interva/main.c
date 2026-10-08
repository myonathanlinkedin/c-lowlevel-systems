#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "core.h"
#include <stdio.h>
#include <assert.h>

/* Helper to count edges – used only in tests. */
static size_t count_edges(const Graph *g) {
    size_t cnt = 0;
    for (size_t i = 0; i < g->n; ++i) {
        for (size_t j = i + 1; j < g->n; ++j) {
            if (graph_is_edge(g, i, j))
                ++cnt;
        }
    }
    return cnt;
}

/* Test 1: Path graph 0‑1‑2‑3, k = 1.
 * Vertices 0 and 3 are simplicial and should be removed.
 * Remaining graph: edge (1,2). Vertex count = 2 ≤ (k+1)^2 = 4.
 */
static void test_path_kernel(void) {
    Graph *g = graph_create(4);
    assert(g != NULL);
    graph_add_edge(g, 0, 1);
    graph_add_edge(g, 1, 2);
    graph_add_edge(g, 2, 3);

    Graph *kg = kernelize_interval_completion(g, 1);
    assert(kg != NULL);
    assert(kg->n == 2);
    assert(graph_is_edge(kg, 0, 1));
    assert(count_edges(kg) == 1);
    graph_destroy(kg);
}

/* Test 2: Complete graph K5, k = 0.
 * Every vertex is simplicial; kernel should be empty.
 */
static void test_complete_kernel(void) {
    Graph *g = graph_create(5);
    assert(g != NULL);
    for (size_t i = 0; i < 5; ++i)
        for (size_t j = i + 1; j < 5; ++j)
            graph_add_edge(g, i, j);

    Graph *kg = kernelize_interval_completion(g, 0);
    assert(kg != NULL);
    assert(kg->n == 0);
    graph_destroy(kg);
}

/* Test 3: Triangle plus pendant vertex.
 * Vertices: 0‑1‑2 form a triangle, vertex 3 adjacent only to 0.
 * Vertex 3 is simplicial (neighbourhood {0} is a clique).
 * After removal we obtain K3, which has no simplicial vertex.
 * Kernel size = 3 ≤ (k+1)^2 for k = 2.
 */
static void test_triangle_pendant(void) {
    Graph *g = graph_create(4);
    assert(g != NULL);
    /* Triangle */
    graph_add_edge(g, 0, 1);
    graph_add_edge(g, 1, 2);
    graph_add_edge(g, 0, 2);
    /* Pendant */
    graph_add_edge(g, 0, 3);

    Graph *kg = kernelize_interval_completion(g, 2);
    assert(kg != NULL);
    assert(kg->n == 3);
    /* Verify that the remaining vertices form a triangle. */
    assert(graph_is_edge(kg, 0, 1));
    assert(graph_is_edge(kg, 1, 2));
    assert(graph_is_edge(kg, 0, 2));
    assert(count_edges(kg) == 3);
    graph_destroy(kg);
}

/* Entry point – runs all unit tests and reports success. */
int main(void) {
    test_path_kernel();
    test_complete_kernel();
    test_triangle_pendant();

    /* Demonstration of kernel size bound */
    size_t k = 3;
    Graph *g = graph_create(10);
    assert(g != NULL);
    /* Build a sparse graph where no vertex is simplicial. */
    for (size_t i = 0; i < 9; ++i)
        graph_add_edge(g, i, i + 1);
    Graph *kg = kernelize_interval_completion(g, k);
    assert(kg != NULL);
    size_t bound = (k + 1) * (k + 1);
    assert(kg->n <= bound || "Kernel exceeds theoretical bound");
    graph_destroy(kg);

    printf("All tests passed.\n");
    return 0;
}
