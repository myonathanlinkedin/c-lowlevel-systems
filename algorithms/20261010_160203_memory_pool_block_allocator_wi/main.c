#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    MemoryPool pool;
    const size_t BLOCK_SIZE = 32;
    const size_t CAPACITY   = 10;

    /* Initialise pool */
    bool ok = mpool_init(&pool, BLOCK_SIZE, CAPACITY);
    assert(ok);
    assert(pool.block_size >= BLOCK_SIZE);
    assert(pool.capacity == CAPACITY);
    assert(pool.used == 0);
    assert(pool.free_list != NULL);

    /* Allocate all blocks */
    void *blocks[CAPACITY];
    for (size_t i = 0; i < CAPACITY; ++i) {
        blocks[i] = mpool_alloc(&pool);
        assert(blocks[i] != NULL);
        /* Verify distinctness */
        for (size_t j = 0; j < i; ++j) {
            assert(blocks[i] != blocks[j]);
        }
    }
    assert(pool.used == CAPACITY);
    assert(pool.free_list == NULL);

    /* Allocation beyond capacity must return NULL */
    void *extra = mpool_alloc(&pool);
    assert(extra == NULL);

    /* Free a subset of blocks */
    for (size_t i = 0; i < CAPACITY; i += 2) {
        mpool_free(&pool, blocks[i]);
        blocks[i] = NULL;
    }
    assert(pool.used == CAPACITY / 2);
    assert(pool.free_list != NULL);

    /* Re‑allocate the freed blocks */
    for (size_t i = 0; i < CAPACITY; i += 2) {
        void *p = mpool_alloc(&pool);
        assert(p != NULL);
        blocks[i] = p;
    }
    assert(pool.used == CAPACITY);
    assert(pool.free_list == NULL);

    /* Free all blocks */
    for (size_t i = 0; i < CAPACITY; ++i) {
        mpool_free(&pool, blocks[i]);
        blocks[i] = NULL;
    }
    assert(pool.used == 0);
    assert(pool.free_list != NULL);

    /* Freeing NULL is a no‑op */
    mpool_free(&pool, NULL);  /* should not crash */

    /* Destroy pool */
    mpool_destroy(&pool);
    assert(pool.buffer == NULL);
    assert(pool.free_list == NULL);
    assert(pool.block_size == 0);
    assert(pool.capacity == 0);
    assert(pool.used == 0);

    /* Demonstration output */
    printf("Memory pool block allocator unit tests passed.\n");
    return 0;
}
