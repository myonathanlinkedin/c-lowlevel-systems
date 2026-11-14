#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t *bits;          // Bit array
    size_t bit_size;        // Number of bits in the filter
    size_t hash_count;      // Number of hash functions (k)
} BloomFilter;

/* Create a Bloom filter sized for `expected_items` with a target false‑positive
 * probability `false_positive_rate` (0 < p < 1). Returns NULL on allocation failure.
 */
BloomFilter* bloom_create(size_t expected_items, double false_positive_rate);

/* Insert `item` (a NUL‑terminated string) into the filter. */
void bloom_add(BloomFilter *bf, const char *item);

/* Test whether `item` is possibly in the filter.
 * Returns true if the item may be present, false if it is definitely absent.
 */
bool bloom_check(const BloomFilter *bf, const char *item);

/* Release all resources associated with the filter. */
void bloom_destroy(BloomFilter *bf);

/* Helper inline functions for bit manipulation */
static inline size_t bloom_bit_index(size_t hash, size_t bit_size) {
    return hash % bit_size;
}
static inline void bloom_set_bit(BloomFilter *bf, size_t idx) {
    bf->bits[idx / 8] |= (uint8_t)(1u << (idx % 8));
}
static inline bool bloom_get_bit(const BloomFilter *bf, size_t idx) {
    return (bf->bits[idx / 8] >> (idx % 8)) & 1u;
}
