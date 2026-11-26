#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include "core.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

static bool double_eq(double a, double b) {
    return fabs(a - b) < TSP_EPS;
}

/* Test: empty graph (n = 0) */
static void test_empty(void) {
    double cost = -1.0;
    uint32_t tour[1];
    int rc = tsp_held_karp(NULL, 0, &cost, tour);
    assert(rc == 0);
    assert(double_eq(cost, 0.0));
    assert(tour[0] == 0);
}

/* Test: single city (n = 1) */
static void test_single(void) {
    double dist[1] = {0.0};
    double cost = -1.0;
    uint32_t tour[2];
    int rc = tsp_held_karp(dist, 1, &cost, tour);
    assert(rc == 0);
    assert(double_eq(cost, 0.0));
    assert(tour[0] == 0 && tour[1] == 0);
}

/* Test: two cities */
static void test_two(void) {
    double dist[4] = {
        0.0, 5.0,
        5.0, 0.0
    };
    double cost = -1.0;
    uint32_t tour[3];
    int rc = tsp_held_karp(dist, 2, &cost, tour);
    assert(rc == 0);
    assert(double_eq(cost, 10.0));
    assert(tour[0] == 0 && tour[1] == 1 && tour[2] == 0);
}

/* Test: triangle (3 cities) */
static void test_triangle(void) {
    /* distances:
       0-1:3, 1-2:4, 0-2:5 (symmetric) */
    double dist[9] = {
        0.0, 3.0, 5.0,
        3.0, 0.0, 4.0,
        5.0, 4.0, 0.0
    };
    double cost = -1.0;
    uint32_t tour[4];
    int rc = tsp_held_karp(dist, 3, &cost, tour);
    assert(rc == 0);
    /* optimal tour 0‑1‑2‑0 = 3+4+5 = 12 */
    assert(double_eq(cost, 12.0));
    assert(tour[0] == 0 && tour[3] == 0);
}

/* Test: square (4 cities) */
static void test_square(void) {
    double d01 = 1.0, d12 = 1.0, d23 = 1.0, d30 = 1.0;
    double d02 = sqrt(2.0), d13 = sqrt(2.0);
    double dist[16] = {
        0.0, d01, d02, d30,
        d01, 0.0, d12, d13,
        d02, d12, 0.0, d23,
        d30, d13, d23, 0.0
    };
    double cost = -1.0;
    uint32_t tour[5];
    int rc = tsp_held_karp(dist, 4, &cost, tour);
    assert(rc == 0);
    /* optimal perimeter tour length = 4.0 */
    assert(double_eq(cost, 4.0));
    assert(tour[0] == 0 && tour[4] == 0);
}

/* Test: asymmetric distances (5 cities) */
static void test_asymmetric(void) {
    /* Simple directed graph where optimal tour is 0→2→1→3→4→0 */
    double dist[25] = {
        0, 2, 9, 10, 7,
        1, 0, 6, 4, 3,
        15, 7, 0, 8, 9,
        6, 3, 12, 0, 5,
        10, 4, 8, 6, 0
    };
    double cost = -1.0;
    uint32_t tour[6];
    int rc = tsp_held_karp(dist, 5, &cost, tour);
    assert(rc == 0);
    /* Known optimal cost = 2+6+4+5+10 = 27 */
    assert(double_eq(cost, 27.0));
    assert(tour[0] == 0 && tour[5] == 0);
}

/* Entry point */
int main(void) {
    test_empty();
    test_single();
    test_two();
    test_triangle();
    test_square();
    test_asymmetric();
    printf("All Held‑Karp TSP tests passed.\n");
    return 0;
}
