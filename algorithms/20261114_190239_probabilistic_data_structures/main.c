#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    /* -------------------------------------------------------------
     * Construct a Bloom filter for 1 000 expected items with a 1 %
     * target false‑positive probability.
     * ------------------------------------------------------------- */
    BloomFilter *bf = bloom_create(1000, 0.01);
    assert(bf != NULL);

    /* -------------------------------------------------------------
     * Test that inserted items are always reported as present.
     * ------------------------------------------------------------- */
    const char *present[] = {
        "alpha", "bravo", "charlie", "delta", "echo",
        "foxtrot", "golf", "hotel", "india", "juliet"
    };
    for (size_t i = 0; i < sizeof(present) / sizeof(present[0]); ++i) {
        bloom_add(bf, present[i]);
    }
    for (size_t i = 0; i < sizeof(present) / sizeof(present[0]); ++i) {
        assert(bloom_check(bf, present[i]) && "Inserted item reported absent");
    }

    /* -------------------------------------------------------------
     * Verify false‑positive behaviour on a disjoint set.
     * With only five queries and a 1 % target rate, at most one false
     * positive is statistically reasonable.
     * ------------------------------------------------------------- */
    const char *absent[] = {
        "kilo", "lima", "mike", "november", "oscar"
    };
    size_t false_positives = 0;
    for (size_t i = 0; i < sizeof(absent) / sizeof(absent[0]); ++i) {
        if (bloom_check(bf, absent[i]))
            ++false_positives;
    }
    assert(false_positives <= 1 && "Unexpectedly high false‑positive count");

    /* -------------------------------------------------------------
     * Edge case: inserting the same element repeatedly must not break
     * the filter.
     * ------------------------------------------------------------- */
    bloom_add(bf, "alpha");
    bloom_add(bf, "alpha");
    assert(bloom_check(bf, "alpha"));

    /* -------------------------------------------------------------
     * Clean up resources.
     * ------------------------------------------------------------- */
    bloom_destroy(bf);
    printf("All Bloom filter unit tests passed.\n");
    return 0;
}
