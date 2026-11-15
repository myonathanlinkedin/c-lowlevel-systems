#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "core.h"
#include "types.h"
#include <assert.h>
#include <stdio.h>

static void test_basic_lru(void) {
    struct Pager p = {0};
    pager_init(&p, 3);
    uint64_t seq[] = {1,2,3,1,4,5};
    for (size_t i = 0; i < sizeof(seq)/sizeof(seq[0]); ++i) {
        pager_access(&p, seq[i]);
    }
    assert(pager_get_faults(&p) == 5);
    assert(pager_get_hits(&p)   == 1);
    assert(pager_get_loaded(&p) == 3);
    pager_destroy(&p);
}

static void test_all_hits(void) {
    struct Pager p = {0};
    pager_init(&p, 2);
    pager_access(&p, 42);
    pager_access(&p, 42);
    pager_access(&p, 42);
    assert(pager_get_faults(&p) == 1);
    assert(pager_get_hits(&p)   == 2);
    pager_destroy(&p);
}

static void test_eviction_order(void) {
    struct Pager p = {0};
    pager_init(&p, 2);
    pager_access(&p, 10); // fault
    pager_access(&p, 20); // fault
    pager_access(&p, 10); // hit, makes 20 LRU
    pager_access(&p, 30); // fault, evicts 20
    assert(pager_get_faults(&p) == 3);
    assert(pager_get_hits(&p)   == 1);
    /* Verify that 20 is no longer present */
    size_t idx = hash_find(&p, 20);
    assert(idx > p.table_mask);
    pager_destroy(&p);
}

static void test_capacity_boundary(void) {
    struct Pager p = {0};
    pager_init(&p, 1);
    pager_access(&p, 1);
    pager_access(&p, 2);
    pager_access(&p, 1);
    assert(pager_get_faults(&p) == 2);
    assert(pager_get_hits(&p)   == 1);
    pager_destroy(&p);
}

int main(void) {
    test_basic_lru();
    test_all_hits();
    test_eviction_order();
    test_capacity_boundary();
    printf("All ChronosPaging unit tests passed.\n");
    return 0;
}
