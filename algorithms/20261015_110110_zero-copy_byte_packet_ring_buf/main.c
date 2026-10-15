#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static void test_basic_push_pop(void) {
    ring_buffer_t rb;
    assert(rb_init(&rb, 4));

    uint8_t a = 1, b = 2, c = 3;
    assert(rb_push(&rb, &a, 1));
    assert(rb_push(&rb, &b, 1));
    assert(rb_size(&rb) == 2);

    uint8_t *out;
    size_t len;
    assert(rb_pop(&rb, &out, &len));
    assert(out == &a && len == 1);
    assert(rb_pop(&rb, &out, &len));
    assert(out == &b && len == 1);
    assert(rb_is_empty(&rb));

    rb_free(&rb);
}

static void test_wrap_around(void) {
    ring_buffer_t rb;
    assert(rb_init(&rb, 2)); // capacity becomes 2 (power of two)

    uint8_t d1 = 10, d2 = 20, d3 = 30;
    assert(rb_push(&rb, &d1, 1));
    assert(rb_push(&rb, &d2, 1));
    assert(!rb_push(&rb, &d3, 1)); // buffer full

    uint8_t *out;
    size_t len;
    assert(rb_pop(&rb, &out, &len));
    assert(out == &d1);
    assert(rb_push(&rb, &d3, 1)); // space after pop

    assert(rb_pop(&rb, &out, &len));
    assert(out == &d2);
    assert(rb_pop(&rb, &out, &len));
    assert(out == &d3);
    assert(rb_is_empty(&rb));

    rb_free(&rb);
}

static void free_handler(uint8_t *data, size_t len) {
    (void)len;
    free(data);
}

static void test_dispatch(void) {
    ring_buffer_t rb;
    assert(rb_init(&rb, 8));

    uint8_t *buf1 = malloc(3);
    buf1[0] = 0xAA; buf1[1] = 0xBB; buf1[2] = 0xCC;
    assert(rb_push(&rb, buf1, 3));

    uint8_t *buf2 = malloc(2);
    buf2[0] = 0x11; buf2[1] = 0x22;
    assert(rb_push(&rb, buf2, 2));

    rb_dispatch(&rb, free_handler);
    assert(rb_is_empty(&rb));

    rb_free(&rb);
}

int main(void) {
    test_basic_push_pop();
    test_wrap_around();
    test_dispatch();
    printf("All tests passed.\n");
    return 0;
}
