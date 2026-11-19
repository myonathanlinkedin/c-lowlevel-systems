#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#include "core.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void test_basic_allocation(void) {
    assert(allocator_init(1024));
    void* p1 = allocator_alloc(100);
    assert(p1 != NULL);
    memset(p1, 0xAA, 100);
    void* p2 = allocator_alloc(200);
    assert(p2 != NULL);
    memset(p2, 0xBB, 200);
    allocator_free(p1);
    allocator_free(p2);
    // After freeing both, a large allocation should succeed
    void* p3 = allocator_alloc(800);
    assert(p3 != NULL);
    allocator_free(p3);
    allocator_destroy();
}

static void test_split_and_coalesce(void) {
    assert(allocator_init(512));
    void* a = allocator_alloc(64);
    void* b = allocator_alloc(64);
    void* c = allocator_alloc(64);
    assert(a && b && c);
    allocator_free(b); // free middle block
    // Allocate a size that fits exactly into the freed block
    void* d = allocator_alloc(64);
    assert(d == b); // should reuse the same spot
    allocator_free(a);
    allocator_free(c);
    // Now three adjacent free blocks should coalesce into one
    void* big = allocator_alloc(200);
    assert(big != NULL);
    allocator_free(d);
    allocator_free(big);
    allocator_destroy();
}

static void test_alignment(void) {
    assert(allocator_init(256));
    for (size_t sz = 1; sz <= 64; ++sz) {
        void* p = allocator_alloc(sz);
        assert(p != NULL);
        // Verify 8‑byte alignment
        assert(((uintptr_t)p & (ALIGNMENT - 1)) == 0);
        allocator_free(p);
    }
    allocator_destroy();
}

static void test_exhaustion(void) {
    assert(allocator_init(128));
    void* p = allocator_alloc(120); // leaves only header space
    assert(p != NULL);
    void* q = allocator_alloc(8); // should fail
    assert(q == NULL);
    allocator_free(p);
    allocator_destroy();
}

int main(void) {
    test_basic_allocation();
    test_split_and_coalesce();
    test_alignment();
    test_exhaustion();
    printf("All allocator tests passed.\n");
    return 0;
}
