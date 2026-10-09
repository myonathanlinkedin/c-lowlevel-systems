#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#include "core.h"
#include <stdio.h>
#include <assert.h>

static void test_basic_allocation(void) {
    MemoryPool pool;
    const size_t block_size = 32;
    const size_t block_count = 10;
    assert(pool_init(&pool, block_size, block_count));

    void *ptrs[block_count];
    // Allocate all blocks
    for (size_t i = 0; i < block_count; ++i) {
        ptrs[i] = pool_alloc(&pool);
        assert(ptrs[i] != NULL);
        // Ensure each pointer lies within the buffer range
        assert((uint8_t*)ptrs[i] >= pool.buffer);
        assert((uint8_t*)ptrs[i] < pool.buffer + pool.block_size * pool.block_count);
    }

    // No more blocks should be available
    assert(pool_alloc(&pool) == NULL);

    // Free in reverse order and reallocate
    for (size_t i = block_count; i-- > 0;) {
        pool_free(&pool, ptrs[i]);
    }

    // Allocate again, should succeed for all blocks
    for (size_t i = 0; i < block_count; ++i) {
        void *p = pool_alloc(&pool);
        assert(p != NULL);
    }

    // Clean up
    pool_destroy(&pool);
    printf("test_basic_allocation passed\n");
}

static void test_partial_free_and_reuse(void) {
    MemoryPool pool;
    const size_t block_size = 24;
    const size_t block_count = 5;
    assert(pool_init(&pool, block_size, block_count));

    void *a = pool_alloc(&pool);
    void *b = pool_alloc(&pool);
    void *c = pool_alloc(&pool);
    assert(a && b && c);

    // Free middle block
    pool_free(&pool, b);

    // Allocate again, should get the same block (b) because LIFO
    void *d = pool_alloc(&pool);
    assert(d == b);

    // Clean up remaining allocations
    pool_free(&pool, a);
    pool_free(&pool, c);
    pool_free(&pool, d);

    pool_destroy(&pool);
    printf("test_partial_free_and_reuse passed\n");
}

static void test_invalid_parameters(void) {
    MemoryPool pool;
    // block size too small
    assert(!pool_init(&pool, sizeof(void*) - 1, 4));
    // zero block count
    assert(!pool_init(&pool, 16, 0));
    // null pool pointer
    assert(!pool_init(NULL, 16, 4));
    printf("test_invalid_parameters passed\n");
}

int main(void) {
    test_basic_allocation();
    test_partial_free_and_reuse();
    test_invalid_parameters();
    printf("All tests passed.\n");
    return 0;
}
