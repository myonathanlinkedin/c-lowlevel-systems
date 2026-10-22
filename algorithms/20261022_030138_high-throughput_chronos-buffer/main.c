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

/* Simple payload for testing */
typedef struct {
    int id;
    char data[16];
} TestPayload;

/* Helper to allocate a payload */
static TestPayload *make_payload(int id) {
    TestPayload *p = (TestPayload *)malloc(sizeof(TestPayload));
    assert(p != NULL);
    p->id = id;
    snprintf(p->data, sizeof(p->data), "msg-%d", id);
    return p;
}

/* Helper to free a payload */
static void free_payload(TestPayload *p) {
    free(p);
}

/* Unit test: basic push/pop */
static void test_basic(void) {
    ChronosRingBuffer rb;
    assert(crb_init(&rb, 8) == true);
    assert(crb_is_empty(&rb));
    assert(!crb_is_full(&rb));

    TestPayload *p1 = make_payload(1);
    assert(crb_push(&rb, p1) == true);
    assert(!crb_is_empty(&rb));
    assert(crb_size(&rb) == 1);

    TestPayload *out = NULL;
    assert(crb_pop(&rb, (void **)&out) == true);
    assert(out == p1);
    assert(out->id == 1);
    free_payload(out);
    assert(crb_is_empty(&rb));
    crb_free(&rb);
}

/* Unit test: fill to capacity and verify full detection */
static void test_full(void) {
    ChronosRingBuffer rb;
    assert(crb_init(&rb, 4) == true);   // capacity 4 => usable slots 3
    TestPayload *items[3];
    for (int i = 0; i < 3; ++i) {
        items[i] = make_payload(i);
        assert(crb_push(&rb, items[i]) == true);
    }
    assert(crb_is_full(&rb));
    /* Attempt to push when full must fail */
    TestPayload *extra = make_payload(99);
    assert(crb_push(&rb, extra) == false);
    free_payload(extra);

    /* Drain all items */
    for (int i = 0; i < 3; ++i) {
        TestPayload *out = NULL;
        assert(crb_pop(&rb, (void **)&out) == true);
        assert(out == items[i]);
        free_payload(out);
    }
    assert(crb_is_empty(&rb));
    crb_free(&rb);
}

/* Unit test: wrap‑around behavior */
static void test_wrap(void) {
    ChronosRingBuffer rb;
    assert(crb_init(&rb, 8) == true);
    /* Fill half */
    for (int i = 0; i < 4; ++i) {
        assert(crb_push(&rb, make_payload(i)) == true);
    }
    /* Pop two */
    for (int i = 0; i < 2; ++i) {
        TestPayload *out = NULL;
        assert(crb_pop(&rb, (void **)&out) == true);
        free_payload(out);
    }
    /* Push four more to force wrap */
    for (int i = 4; i < 8; ++i) {
        assert(crb_push(&rb, make_payload(i)) == true);
    }
    /* Verify size */
    assert(crb_size(&rb) == 6);
    /* Drain remaining */
    while (!crb_is_empty(&rb)) {
        TestPayload *out = NULL;
        assert(crb_pop(&rb, (void **)&out) == true);
        free_payload(out);
    }
    crb_free(&rb);
}

/* Unit test: invalid capacity handling */
static void test_invalid_capacity(void) {
    ChronosRingBuffer rb;
    /* Non‑power‑of‑two should be rejected */
    assert(crb_init(&rb, 7) == false);
}

/* Benchmark: simple throughput measurement (non‑precise) */
static void benchmark(void) {
    const size_t cap = 1 << 14;   // 16384 slots
    ChronosRingBuffer rb;
    assert(crb_init(&rb, cap) == true);
    const size_t iterations = 1e6;
    for (size_t i = 0; i < iterations; ++i) {
        TestPayload *p = make_payload((int)i);
        while (!crb_push(&rb, p)) {
            /* spin until space available */
        }
        TestPayload *out = NULL;
        while (!crb_pop(&rb, (void **)&out)) {
            /* spin until item available */
        }
        free_payload(out);
    }
    crb_free(&rb);
    printf("Benchmark completed: %zu iterations on capacity %zu\n", iterations, cap);
}

int main(void) {
    test_basic();
    test_full();
    test_wrap();
    test_invalid_capacity();
    benchmark();
    printf("All tests passed.\n");
    return 0;
}
