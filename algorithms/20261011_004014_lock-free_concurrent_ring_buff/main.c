#include "types.h"
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#include "core.h"
#include <stdio.h>
#include <assert.h>
#include <threads.h>

#define TEST_CAPACITY  8
#define PRODUCE_COUNT 1000000

/* ---------- Single‑threaded correctness tests ---------- */
static void test_basic(void) {
    ring_buffer_t rb;
    assert(ring_init(&rb, TEST_CAPACITY));
    assert(rb.capacity >= TEST_CAPACITY);
    /* Fill the buffer */
    for (size_t i = 0; i < rb.capacity; ++i) {
        assert(ring_push(&rb, (int)i));
    }
    /* Buffer should now be full */
    assert(!ring_push(&rb, 999));

    /* Drain the buffer */
    for (size_t i = 0; i < rb.capacity; ++i) {
        int v = -1;
        assert(ring_pop(&rb, &v));
        assert(v == (int)i);
    }
    /* Buffer should now be empty */
    int dummy;
    assert(!ring_pop(&rb, &dummy));

    ring_free(&rb);
}

/* ---------- Multi‑threaded SPSC stress test ---------- */
typedef struct {
    ring_buffer_t *rb;
    size_t count;
} thread_arg_t;

static int producer(void *arg) {
    thread_arg_t *a = (thread_arg_t *)arg;
    for (size_t i = 0; i < a->count; ++i) {
        /* Spin until we can push */
        while (!ring_push(a->rb, (int)i)) {
            thrd_yield();
        }
    }
    return 0;
}

static int consumer(void *arg) {
    thread_arg_t *a = (thread_arg_t *)arg;
    size_t received = 0;
    while (received < a->count) {
        int v;
        if (ring_pop(a->rb, &v)) {
            assert(v == (int)received);
            ++received;
        } else {
            thrd_yield();
        }
    }
    return 0;
}

static void test_spsc(void) {
    ring_buffer_t rb;
    assert(ring_init(&rb, 1024));
    thread_arg_t parg = { .rb = &rb, .count = PRODUCE_COUNT };
    thread_arg_t carg = { .rb = &rb, .count = PRODUCE_COUNT };
    thrd_t prod, cons;
    assert(thrd_create(&prod, producer, &parg) == thrd_success);
    assert(thrd_create(&cons, consumer, &carg) == thrd_success);
    thrd_join(prod, NULL);
    thrd_join(cons, NULL);
    /* After both threads finish the buffer must be empty */
    int dummy;
    assert(!ring_pop(&rb, &dummy));
    ring_free(&rb);
}

/* ---------- Entry point ---------- */
int main(void) {
    printf("Running single‑threaded tests...\\n");
    test_basic();
    printf("Single‑threaded tests passed.\\n");

    printf("Running multi‑threaded SPSC stress test...\\n");
    test_spsc();
    printf("Multi‑threaded test passed.\\n");

    printf("All tests completed successfully.\\n");
    return 0;
}
