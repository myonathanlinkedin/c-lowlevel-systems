#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>

#include "core.h"
#include "types.h"
#include <stdio.h>
#include <assert.h>

static void test_basic_allocation(void) {
    MemoryPool pool;
    assert(mpool_init(&pool, 32, 4) == true);
    void *a = mpool_alloc(&pool);
    void *b = mpool_alloc(&pool);
    void *c = mpool_alloc(&pool);
    void *d = mpool_alloc(&pool);
    void *e = mpool_alloc(&pool);   // should be NULL (exhausted)

    assert(a != NULL && b != NULL && c != NULL && d != NULL);
    assert(e == NULL);

    /* Free one block and allocate again */
    assert(mpool_free(&pool, b) == true);
    void *f = mpool_alloc(&pool);
    assert(f == b);                 // should reuse the same block
    assert(mpool_free(&pool, a) == true);
    assert(mpool_free(&pool, c) == true);
    assert(mpool_free(&pool, d) == true);
    assert(mpool_free(&pool, f) == true);
    mpool_destroy(&pool);
}

static void test_invalid_operations(void) {
    MemoryPool pool;
    assert(mpool_init(&pool, 24, 2) == true);
    void *p1 = mpool_alloc(&pool);
    void *p2 = mpool_alloc(&pool);
    assert(p1 && p2);
    /* Double free */
    assert(mpool_free(&pool, p1) == true);
    assert(mpool_free(&pool, p1) == false);
    /* Free pointer not from pool */
    int external;
    assert(mpool_free(&pool, &external) == false);
    /* Free NULL */
    assert(mpool_free(&pool, NULL) == false);
    /* Allocate after freeing one block */
    void *p3 = mpool_alloc(&pool);
    assert(p3 == p1);
    mpool_destroy(&pool);
}

static void test_edge_cases(void) {
    MemoryPool pool;
    /* Zero block size or count must fail */
    assert(mpool_init(&pool, 0, 5) == false);
    assert(mpool_init(&pool, 16, 0) == false);
    /* Very small block size (less than pointer size) is rounded up */
    assert(mpool_init(&pool, 1, 3) == true);
    void *x = mpool_alloc(&pool);
    assert(x != NULL);
    assert(mpool_free(&pool, x) == true);
    mpool_destroy(&pool);
}

int main(void) {
    test_basic_allocation();
    test_invalid_operations();
    test_edge_cases();
    printf("All MemoryPool tests passed.\n");
    return 0;
}
