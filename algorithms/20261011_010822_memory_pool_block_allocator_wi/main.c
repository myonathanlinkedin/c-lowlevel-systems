#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "types.h"

#include <stdio.h>
#include <assert.h>

#define TEST_BLOCK_SIZE  32U
#define TEST_BLOCK_COUNT 128U

int main(void) {
    MemoryPool pool;
    bool ok = pool_init(&pool, TEST_BLOCK_SIZE, TEST_BLOCK_COUNT);
    assert(ok && "Pool initialisation must succeed");

    /* Allocate all blocks */
    void *ptrs[TEST_BLOCK_COUNT];
    for (size_t i = 0U; i < TEST_BLOCK_COUNT; ++i) {
        ptrs[i] = pool_alloc(&pool);
        assert(ptrs[i] != NULL && "Allocation should succeed while pool not exhausted");
    }

    /* Pool should now be empty */
    assert(pool_alloc(&pool) == NULL && "Further allocation must return NULL");

    /* Verify uniqueness of allocated pointers */
    for (size_t i = 0U; i < TEST_BLOCK_COUNT; ++i) {
        for (size_t j = i + 1U; j < TEST_BLOCK_COUNT; ++j) {
            assert(ptrs[i] != ptrs[j] && "All allocated pointers must be distinct");
        }
    }

    /* Free every block in reverse order */
    for (size_t i = TEST_BLOCK_COUNT; i-- > 0U;) {
        pool_free(&pool, ptrs[i]);
    }

    /* After freeing, we should be able to allocate again */
    for (size_t i = 0U; i < TEST_BLOCK_COUNT; ++i) {
        void *p = pool_alloc(&pool);
        assert(p != NULL && "Re‑allocation after free must succeed");
        /* Optional: write a pattern to ensure memory is usable */
        memset(p, 0xA5, TEST_BLOCK_SIZE);
    }

    /* Clean up */
    pool_destroy(&pool);
    printf("All memory pool tests passed.\n");
    return 0;
}
