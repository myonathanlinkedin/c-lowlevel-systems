#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void test_basic_allocation(void) {
    uint8_t backing[1024];
    Arena a;
    arena_init(&a, backing, sizeof(backing));

    void *p1 = arena_alloc(&a, 100, 8);
    assert(p1 != NULL);
    assert(((uintptr_t)p1 & (8 - 1)) == 0);
    memset(p1, 0xAA, 100);

    void *p2 = arena_alloc(&a, 200, 16);
    assert(p2 != NULL);
    assert(((uintptr_t)p2 & (16 - 1)) == 0);
    memset(p2, 0xBB, 200);

    /* Verify that allocations do not overlap. */
    assert((uint8_t *)p2 >= (uint8_t *)p1 + 100);
}

static void test_alignment(void) {
    uint8_t backing[256];
    Arena a;
    arena_init(&a, backing, sizeof(backing));

    for (size_t align = 1; align <= 64; align <<= 1) {
        void *p = arena_alloc(&a, 1, align);
        assert(p != NULL);
        assert(((uintptr_t)p & (align - 1)) == 0);
    }
}

static void test_overflow(void) {
    uint8_t backing[128];
    Arena a;
    arena_init(&a, backing, sizeof(backing));

    void *p = arena_alloc(&a, 128, 1);
    assert(p != NULL);

    void *q = arena_alloc(&a, 1, 1);
    assert(q == NULL); /* No space left */
}

static void test_reset(void) {
    uint8_t backing[256];
    Arena a;
    arena_init(&a, backing, sizeof(backing));

    void *p1 = arena_alloc(&a, 64, 8);
    assert(p1 != NULL);
    arena_reset(&a);
    void *p2 = arena_alloc(&a, 128, 8);
    assert(p2 != NULL);
    assert(p2 == backing); /* After reset we start from base */
}

static void test_owned_buffer(void) {
    Arena a;
    bool ok = arena_init_owned(&a, 512);
    assert(ok);
    void *p = arena_alloc(&a, 256, 8);
    assert(p != NULL);
    arena_destroy(&a);
    /* After destroy the arena should be inert; further alloc must fail. */
    void *q = arena_alloc(&a, 1, 1);
    assert(q == NULL);
}

int main(void) {
    test_basic_allocation();
    test_alignment();
    test_overflow();
    test_reset();
    test_owned_buffer();

    printf("All arena allocator tests passed.\n");
    return 0;
}
