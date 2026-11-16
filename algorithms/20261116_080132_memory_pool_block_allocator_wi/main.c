#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <stdalign.h>

int main(void) {
    // Test 1: Basic allocation and deallocation
    {
        Pool pool;
        bool ok = pool_init(&pool, 32, 10);
        assert(ok);
        void *ptrs[10];
        for (size_t i = 0; i < 10; ++i) {
            ptrs[i] = pool_alloc(&pool);
            assert(ptrs[i] != NULL);
        }
        // Pool should be exhausted
        assert(pool_alloc(&pool) == NULL);
        // Free all blocks
        for (size_t i = 0; i < 10; ++i) {
            pool_free(&pool, ptrs[i]);
        }
        // Allocate again
        for (size_t i = 0; i < 10; ++i) {
            ptrs[i] = pool_alloc(&pool);
            assert(ptrs[i] != NULL);
        }
        pool_destroy(&pool);
    }

    // Test 2: Zero block size should fail
    {
        Pool pool;
        bool ok = pool_init(&pool, 0, 10);
        assert(!ok);
    }

    // Test 3: Zero block count should fail
    {
        Pool pool;
        bool ok = pool_init(&pool, 32, 0);
        assert(!ok);
    }

    // Test 4: Alignment correctness
    {
        Pool pool;
        bool ok = pool_init(&pool, 15, 5);
        assert(ok);
        void *ptr = pool_alloc(&pool);
        assert(ptr != NULL);
        assert(((uintptr_t)ptr % alignof(max_align_t)) == 0);
        pool_free(&pool, ptr);
        pool_destroy(&pool);
    }

    // Test 5: Large pool
    {
        Pool pool;
        const size_t block_size = 64;
        const size_t block_count = 1000;
        bool ok = pool_init(&pool, block_size, block_count);
        assert(ok);
        void *ptrs[block_count];
        for (size_t i = 0; i < block_count; ++i) {
            ptrs[i] = pool_alloc(&pool);
            assert(ptrs[i] != NULL);
        }
        // Exhausted
        assert(pool_alloc(&pool) == NULL);
        // Free all
        for (size_t i = 0; i < block_count; ++i) {
            pool_free(&pool, ptrs[i]);
        }
        // Allocate again
        for (size_t i = 0; i < block_count; ++i) {
            ptrs[i] = pool_alloc(&pool);
            assert(ptrs[i] != NULL);
        }
        pool_destroy(&pool);
    }

    // Test 6: Repeated free and allocate
    {
        Pool pool;
        bool ok = pool_init(&pool, 32, 3);
        assert(ok);
        void *a = pool_alloc(&pool);
        void *b = pool_alloc(&pool);
        void *c = pool_alloc(&pool);
        assert(a && b && c);
        pool_free(&pool, b);
        pool_free(&pool, a);
        pool_free(&pool, c);
        // All blocks should be available again
        void *x = pool_alloc(&pool);
        void *y = pool_alloc(&pool);
        void *z = pool_alloc(&pool);
        assert(x && y && z);
        pool_destroy(&pool);
    }

    printf("All tests passed.\n");
    return 0;
}
