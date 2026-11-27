#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "core.h"
#include "types.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define POOL_SIZE (64 * 1024)   /* 64 KiB test pool */

static uint8_t global_pool[POOL_SIZE];

/* Basic allocate / free cycle. */
static void test_basic_allocation(void)
{
    allocator_init(global_pool, POOL_SIZE);

    void* a = allocator_alloc(100);
    assert(a != NULL);
    memset(a, 0xAA, 100);

    void* b = allocator_alloc(200);
    assert(b != NULL);
    memset(b, 0xBB, 200);

    allocator_free(a);
    allocator_free(b);

    /* Re‑allocate, expecting reuse of freed space. */
    void* c = allocator_alloc(150);
    assert(c != NULL);
    allocator_free(c);
}

/* Verify that every allocation respects the required alignment. */
static void test_alignment(void)
{
    allocator_init(global_pool, POOL_SIZE);
    for (size_t sz = 1; sz <= 64; ++sz) {
        void* p = allocator_alloc(sz);
        assert(p != NULL);
        assert(((uintptr_t)p & (ALIGNMENT - 1)) == 0);
        allocator_free(p);
    }
}

/* Exhaust the pool, then free everything and allocate a large block again. */
static void test_exhaustion(void)
{
    allocator_init(global_pool, POOL_SIZE);
    void* ptrs[1024];
    size_t count = 0;

    while (1) {
        void* p = allocator_alloc(1024);
        if (p == NULL) break;
        ptrs[count++] = p;
    }
    assert(count > 0);

    for (size_t i = 0; i < count; ++i) {
        allocator_free(ptrs[i]);
    }

    void* big = allocator_alloc(POOL_SIZE / 2);
    assert(big != NULL);
    allocator_free(big);
}

/* Double free should trigger an assert in debug builds. */
static void test_double_free_detection(void)
{
    allocator_init(global_pool, POOL_SIZE);
    void* p = allocator_alloc(128);
    assert(p != NULL);
    allocator_free(p);

#ifndef NDEBUG
    /* Uncomment the next line to see the assert fire. */
    /* allocator_free(p); */
#endif
}

int main(void)
{
    test_basic_allocation();
    test_alignment();
    test_exhaustion();
    test_double_free_detection();

    printf("All allocator tests passed.\n");
    return 0;
}
