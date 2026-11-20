#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "core.h"
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

#define TEST_CAPACITY 8
#define TEST_ITERATIONS 100000

// Simple integer wrapper to avoid pointer aliasing issues
static inline aether_item_t int_to_item(int v) { return (aether_item_t)(intptr_t)v; }
static inline int item_to_int(aether_item_t i) { return (int)(intptr_t)i; }

// Single‑threaded correctness tests
static void test_basic_operations(void) {
    aether_ring_t rb;
    assert(aether_ring_init(&rb, TEST_CAPACITY));
    assert(aether_ring_is_empty(&rb));
    assert(!aether_ring_is_full(&rb));

    // Fill the buffer
    for (int i = 0; i < (int)rb.capacity; ++i) {
        assert(aether_ring_push(&rb, int_to_item(i)));
    }
    assert(aether_ring_is_full(&rb));
    assert(!aether_ring_push(&rb, int_to_item(999))); // overflow

    // Drain the buffer
    for (int i = 0; i < (int)rb.capacity; ++i) {
        aether_item_t out;
        assert(aether_ring_pop(&rb, &out));
        assert(item_to_int(out) == i);
    }
    assert(aether_ring_is_empty(&rb));
    assert(!aether_ring_pop(&rb, &(aether_item_t){0})); // underflow

    aether_ring_destroy(&rb);
}

// Multi‑threaded SPSC stress test
typedef struct {
    aether_ring_t *rb;
    int start;
    int count;
} thread_arg_t;

static void *producer_thread(void *arg) {
    thread_arg_t *parg = (thread_arg_t *)arg;
    for (int i = 0; i < parg->count; ++i) {
        // spin until we can push
        while (!aether_ring_push(parg->rb, int_to_item(parg->start + i))) {
            // busy‑wait
        }
    }
    return NULL;
}

static void *consumer_thread(void *arg) {
    thread_arg_t *parg = (thread_arg_t *)arg;
    int received = 0;
    while (received < parg->count) {
        aether_item_t out;
        if (aether_ring_pop(parg->rb, &out)) {
            int val = item_to_int(out);
            // Simple verification: values should be within expected range
            assert(val >= parg->start && val < parg->start + parg->count);
            ++received;
        }
    }
    return NULL;
}

static void test_spsc_concurrency(void) {
    aether_ring_t rb;
    assert(aether_ring_init(&rb, 1024));

    thread_arg_t prod = { .rb = &rb, .start = 0, .count = TEST_ITERATIONS };
    thread_arg_t cons = { .rb = &rb, .start = 0, .count = TEST_ITERATIONS };

    pthread_t th_prod, th_cons;
    assert(pthread_create(&th_prod, NULL, producer_thread, &prod) == 0);
    assert(pthread_create(&th_cons, NULL, consumer_thread, &cons) == 0);

    pthread_join(th_prod, NULL);
    pthread_join(th_cons, NULL);

    assert(aether_ring_is_empty(&rb));
    aether_ring_destroy(&rb);
}

// Simple benchmark (single‑threaded)
static void benchmark_push_pop(void) {
    aether_ring_t rb;
    const size_t cap = 1 << 20; // ~1M entries
    assert(aether_ring_init(&rb, cap));

    const size_t ops = 10 * 1000 * 1000;
    for (size_t i = 0; i < ops; ++i) {
        while (!aether_ring_push(&rb, int_to_item((int)i))) { }
        aether_item_t out;
        while (!aether_ring_pop(&rb, &out)) { }
    }
    printf("Benchmark completed: %zu push/pop cycles on capacity %zu\n", ops, cap);
    aether_ring_destroy(&rb);
}

int main(void) {
    printf("Running single‑threaded correctness tests...\n");
    test_basic_operations();
    printf("Single‑threaded tests passed.\n");

    printf("Running SPSC concurrency test...\n");
    test_spsc_concurrency();
    printf("Concurrency test passed.\n");

    printf("Running simple benchmark...\n");
    benchmark_push_pop();

    printf("All tests and benchmark completed successfully.\n");
    return 0;
}
