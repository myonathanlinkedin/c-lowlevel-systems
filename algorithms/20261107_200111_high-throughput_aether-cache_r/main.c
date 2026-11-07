#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#include "types.h"
#include "core.h"
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

static void test_basic_operations(void) {
    ring_t r;
    assert(ring_init(&r, 8));               // usable slots = 7
    assert(ring_is_empty(&r));
    assert(!ring_is_full(&r));

    int a = 1, b = 2, c = 3;
    assert(ring_push(&r, &a));
    assert(!ring_is_empty(&r));
    assert(!ring_is_full(&r));

    void *p = NULL;
    assert(ring_pop(&r, &p));
    assert(p == &a);
    assert(ring_is_empty(&r));

    // Fill to capacity
    for (int i = 0; i < 7; ++i) {
        assert(ring_push(&r, (void *)(intptr_t)i));
    }
    assert(ring_is_full(&r));
    assert(!ring_push(&r, &b)); // should reject

    // Drain all
    for (int i = 0; i < 7; ++i) {
        assert(ring_pop(&r, &p));
        assert(p == (void *)(intptr_t)i);
    }
    assert(ring_is_empty(&r));
    assert(!ring_pop(&r, &p));

    ring_free(&r);
}

static void test_wrap_around(void) {
    ring_t r;
    assert(ring_init(&r, 4)); // usable slots = 3
    int vals[6] = {10, 20, 30, 40, 50, 60};

    // Fill three slots
    assert(ring_push(&r, &vals[0]));
    assert(ring_push(&r, &vals[1]));
    assert(ring_push(&r, &vals[2])); // now full

    void *p = NULL;
    assert(ring_pop(&r, &p));
    assert(p == &vals[0]);
    assert(ring_pop(&r, &p));
    assert(p == &vals[1]);

    // Two slots free, push two more (wrap)
    assert(ring_push(&r, &vals[3]));
    assert(ring_push(&r, &vals[4]));
    assert(ring_is_full(&r));

    // Drain remaining
    assert(ring_pop(&r, &p));
    assert(p == &vals[2]);
    assert(ring_pop(&r, &p));
    assert(p == &vals[3]);
    assert(ring_pop(&r, &p));
    assert(p == &vals[4]);
    assert(ring_is_empty(&r));

    ring_free(&r);
}

int main(void) {
    test_basic_operations();
    test_wrap_around();
    printf("All tests passed.\n");
    return 0;
}
