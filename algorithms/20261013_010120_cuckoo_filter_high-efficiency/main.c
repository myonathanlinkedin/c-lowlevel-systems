#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Helper to generate a deterministic integer key */
static uint64_t make_key(uint64_t i)
{
    return i;
}

/* Unit tests */
static void test_basic_operations(void)
{
    CuckooFilter *cf = cf_create(1024, 4, 8);
    assert(cf != NULL);

    /* Insert a range of keys */
    for (uint64_t i = 0; i < 500; ++i) {
        uint64_t key = make_key(i);
        assert(cf_insert(cf, &key, sizeof(key)) == true);
        assert(cf_contains(cf, &key, sizeof(key)) == true);
    }

    /* Verify that all inserted keys are reported present */
    for (uint64_t i = 0; i < 500; ++i) {
        uint64_t key = make_key(i);
        assert(cf_contains(cf, &key, sizeof(key)) == true);
    }

    /* Delete a subset and verify absence */
    for (uint64_t i = 0; i < 200; ++i) {
        uint64_t key = make_key(i);
        assert(cf_delete(cf, &key, sizeof(key)) == true);
        assert(cf_contains(cf, &key, sizeof(key)) == false);
    }

    /* Ensure remaining keys are still present */
    for (uint64_t i = 200; i < 500; ++i) {
        uint64_t key = make_key(i);
        assert(cf_contains(cf, &key, sizeof(key)) == true);
    }

    cf_destroy(cf);
}

static void test_false_positive_rate(void)
{
    CuckooFilter *cf = cf_create(2048, 4, 8);
    assert(cf != NULL);

    const size_t N = 1000;
    for (size_t i = 0; i < N; ++i) {
        uint64_t key = make_key(i * 3);
        assert(cf_insert(cf, &key, sizeof(key)) == true);
    }

    size_t false_positives = 0;
    size_t trials = 5000;
    for (size_t i = 0; i < trials; ++i) {
        uint64_t key = make_key(1000000ULL + i); /* keys not inserted */
        if (cf_contains(cf, &key, sizeof(key))) {
            ++false_positives;
        }
    }

    double fp_rate = (double)false_positives / (double)trials;
    /* Expected false‑positive rate for 8‑bit fingerprint ≈ 1/256 ≈ 0.004 */
    assert(fp_rate < 0.02); /* generous bound for deterministic test */

    cf_destroy(cf);
}

static void test_full_filter_behavior(void)
{
    /* Small filter to force insertion failure */
    CuckooFilter *cf = cf_create(32, 2, 8);
    assert(cf != NULL);

    size_t inserted = 0;
    for (size_t i = 0; i < 1000; ++i) {
        uint64_t key = make_key(i);
        if (cf_insert(cf, &key, sizeof(key))) {
            ++inserted;
        } else {
            break;
        }
    }
    /* At least some insertions must have succeeded */
    assert(inserted > 0);
    /* After failure, the filter should still correctly answer queries for inserted items */
    for (size_t i = 0; i < inserted; ++i) {
        uint64_t key = make_key(i);
        assert(cf_contains(cf, &key, sizeof(key)) == true);
    }

    cf_destroy(cf);
}

int main(void)
{
    test_basic_operations();
    test_false_positive_rate();
    test_full_filter_behavior();

    printf("All Cuckoo Filter unit tests passed.\n");
    return 0;
}
