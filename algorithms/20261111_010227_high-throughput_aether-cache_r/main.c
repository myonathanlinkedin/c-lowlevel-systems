#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#include "core.h"
#include <stdio.h>
#include <assert.h>

/* Simple deterministic unit tests for the ring buffer. */
static void test_basic_push_pop(void) {
    ring_t r;
    assert(ring_init(&r, 8));
    int a = 1, b = 2, c = 3;
    assert(ring_is_empty(&r));
    assert(!ring_is_full(&r));

    assert(ring_push(&r, &a));
    assert(!ring_is_empty(&r));
    assert(!ring_is_full(&r));

    void *out = NULL;
    assert(ring_pop(&r, &out));
    assert(out == &a);
    assert(ring_is_empty(&r));

    /* Fill the buffer completely */
    for (int i = 0; i < 8; ++i) {
        int *p = (int *)malloc(sizeof(int));
        *p = i;
        assert(ring_push(&r, p));
    }
    assert(ring_is_full(&r));
    assert(!ring_push(&r, &b)); // should reject

    /* Empty it */
    for (int i = 0; i < 8; ++i) {
        void *ptr = NULL;
        assert(ring_pop(&r, &ptr));
        int *val = (int *)ptr;
        assert(*val == i);
        free(val);
    }
    assert(ring_is_empty(&r));
    assert(!ring_pop(&r, &out)); // nothing to pop

    ring_free(&r);
}

static void test_wrap_around(void) {
    ring_t r;
    assert(ring_init(&r, 4)); // capacity 4, mask 3

    int vals[4] = {10, 20, 30, 40};
    for (int i = 0; i < 4; ++i) {
        assert(ring_push(&r, &vals[i]));
    }
    assert(ring_is_full(&r));

    void *out = NULL;
    assert(ring_pop(&r, &out));
    assert(out == &vals[0]);
    assert(!ring_is_full(&r));

    int extra = 50;
    assert(ring_push(&r, &extra));
    assert(ring_is_full(&r));

    /* Now order should be 20,30,40,50 */
    int *expected[] = {&vals[1], &vals[2], &vals[3], &extra};
    for (int i = 0; i < 4; ++i) {
        assert(ring_pop(&r, &out));
        assert(out == expected[i]);
    }
    assert(ring_is_empty(&r));
    ring_free(&r);
}

/* Stress test: push/pop alternating many times to exercise wrap logic. */
static void test_stress(void) {
    const size_t cap = 1024;
    ring_t r;
    assert(ring_init(&r, cap));

    const size_t iterations = 100000;
    for (size_t i = 0; i < iterations; ++i) {
        int *p = (int *)malloc(sizeof(int));
        *p = (int)i;
        assert(ring_push(&r, p));
        void *out = NULL;
        assert(ring_pop(&r, &out));
        int *val = (int *)out;
        assert(*val == (int)i);
        free(val);
    }
    assert(ring_is_empty(&r));
    ring_free(&r);
}

int main(void) {
    test_basic_push_pop();
    test_wrap_around();
    test_stress();
    printf("All ring buffer tests passed.\n");
    return 0;
}
