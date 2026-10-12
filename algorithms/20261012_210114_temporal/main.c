#include "types.h"
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "core.h"
#include <stdio.h>
#include <assert.h>

static void test_basic_operations(void) {
    TemporalQueue* q = tq_create();
    assert(q != NULL);
    assert(tq_size(q) == 0);

    // Insert a few items with out‑of‑order timestamps.
    assert(tq_push(q, 100, 1));
    assert(tq_push(q, 50, 2));
    assert(tq_push(q, 150, 3));
    assert(tq_push(q, 75, 4));
    assert(tq_size(q) == 4);

    // Peek should give the smallest timestamp (50).
    uint64_t ts; int val;
    assert(tq_peek(q, &ts, &val));
    assert(ts == 50 && val == 2);

    // Pop items and verify order.
    assert(tq_pop(q, &ts, &val));
    assert(ts == 50 && val == 2);
    assert(tq_pop(q, &ts, &val));
    assert(ts == 75 && val == 4);
    assert(tq_pop(q, &ts, &val));
    assert(ts == 100 && val == 1);
    assert(tq_pop(q, &ts, &val));
    assert(ts == 150 && val == 3);
    assert(!tq_pop(q, &ts, &val)); // now empty

    tq_destroy(q);
}

static void test_purge_before(void) {
    TemporalQueue* q = tq_create();
    assert(q != NULL);
    // Insert timestamps 10,20,30,40,50
    for (uint64_t i = 1; i <= 5; ++i) {
        assert(tq_push(q, i * 10, (int)i));
    }
    assert(tq_size(q) == 5);

    // Purge everything before 35 -> should remove 10,20,30
    tq_purge_before(q, 35);
    assert(tq_size(q) == 2);

    uint64_t ts; int val;
    assert(tq_peek(q, &ts, &val));
    assert(ts == 40 && val == 4);
    assert(tq_pop(q, &ts, &val));
    assert(ts == 40 && val == 4);
    assert(tq_pop(q, &ts, &val));
    assert(ts == 50 && val == 5);
    assert(!tq_pop(q, &ts, &val));

    tq_destroy(q);
}

static void test_edge_cases(void) {
    TemporalQueue* q = tq_create();
    assert(q != NULL);
    // Pop/peek on empty queue must fail.
    uint64_t ts; int val;
    assert(!tq_peek(q, &ts, &val));
    assert(!tq_pop(q, &ts, &val));

    // Insert duplicate timestamps.
    assert(tq_push(q, 42, 100));
    assert(tq_push(q, 42, 200));
    assert(tq_size(q) == 2);

    // Order between duplicates is unspecified but both must be present.
    assert(tq_pop(q, &ts, &val));
    assert(ts == 42);
    assert(tq_pop(q, &ts, &val));
    assert(ts == 42);
    assert(!tq_pop(q, &ts, &val));

    // Purge on empty queue must be safe.
    tq_purge_before(q, 100);
    assert(tq_size(q) == 0);

    tq_destroy(q);
}

int main(void) {
    test_basic_operations();
    test_purge_before();
    test_edge_cases();

    // Simple benchmark (deterministic, small scale)
    TemporalQueue* bench = tq_create();
    const size_t N = 10000;
    for (size_t i = 0; i < N; ++i) {
        // Insert timestamps in reverse order to stress heapify_up.
        assert(tq_push(bench, N - i, (int)i));
    }
    assert(tq_size(bench) == N);
    // Remove all items; timestamps should come out ascending.
    uint64_t prev = 0;
    for (size_t i = 0; i < N; ++i) {
        uint64_t ts; int v;
        assert(tq_pop(bench, &ts, &v));
        assert(ts == i + 1); // because we inserted N..1
        (void)v; // silence unused warning
        prev = ts;
    }
    assert(tq_size(bench) == 0);
    tq_destroy(bench);

    printf("All temporal queue tests passed.\n");
    return 0;
}
