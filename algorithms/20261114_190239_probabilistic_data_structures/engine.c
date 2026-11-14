#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

/* -------------------------------------------------------------------------
 * Simple, deterministic hash functions (djb2 and sdbm). They operate on
 * unsigned char values to avoid sign‑extension issues.
 * ------------------------------------------------------------------------- */
static uint32_t hash_djb2(const char *str) {
    uint32_t hash = 5381u;
    int c;
    while ((c = (unsigned char)*str++) != 0) {
        hash = ((hash << 5) + hash) + (uint32_t)c;   // hash * 33 + c
    }
    return hash;
}
static uint32_t hash_sdbm(const char *str) {
    uint32_t hash = 0u;
    int c;
    while ((c = (unsigned char)*str++) != 0) {
        hash = (uint32_t)c + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}

/* Double‑hashing scheme:  h_i(x) = h1(x) + i·h2(x) + i²  (mod m)   */
static uint32_t bloom_hash(const char *item, size_t i) {
    uint32_t h1 = hash_djb2(item);
    uint32_t h2 = hash_sdbm(item);
    return h1 + (uint32_t)i * h2 + (uint32_t)i * (uint32_t)i;
}

/* -------------------------------------------------------------------------
 * Bloom filter construction based on classic formulas:
 *   m = -(n·ln(p)) / (ln 2)²   (bits)
 *   k = (m/n)·ln 2            (hash functions)
 * ------------------------------------------------------------------------- */
BloomFilter* bloom_create(size_t expected_items, double false_positive_rate) {
    assert(expected_items > 0);
    assert(false_positive_rate > 0.0 && false_positive_rate < 1.0);

    const double ln2 = 0.6931471805599453;
    double m = -(double)expected_items * log(false_positive_rate) / (ln2 * ln2);
    size_t bit_size = (size_t)ceil(m);
    double k = (m / (double)expected_items) * ln2;
    size_t hash_count = (size_t)ceil(k);
    if (hash_count == 0) hash_count = 1;   // safeguard against pathological rounding

    size_t byte_len = (bit_size + 7) / 8;
    uint8_t *bits = (uint8_t *)calloc(byte_len, 1);
    if (!bits) return NULL;

    BloomFilter *bf = (BloomFilter *)malloc(sizeof *bf);
    if (!bf) {
        free(bits);
        return NULL;
    }
    bf->bits = bits;
    bf->bit_size = bit_size;
    bf->hash_count = hash_count;
    return bf;
}

/* Insert an element by setting the k bits derived from the hash functions. */
void bloom_add(BloomFilter *bf, const char *item) {
    assert(bf != NULL && item != NULL);
    for (size_t i = 0; i < bf->hash_count; ++i) {
        uint32_t h = bloom_hash(item, i);
        size_t idx = bloom_bit_index((size_t)h, bf->bit_size);
        bloom_set_bit(bf, idx);
    }
}

/* Query an element: all k bits must be set for a positive answer. */
bool bloom_check(const BloomFilter *bf, const char *item) {
    assert(bf != NULL && item != NULL);
    for (size_t i = 0; i < bf->hash_count; ++i) {
        uint32_t h = bloom_hash(item, i);
        size_t idx = bloom_bit_index((size_t)h, bf->bit_size);
        if (!bloom_get_bit(bf, idx))
            return false;
    }
    return true;
}

/* Release allocated memory. */
void bloom_destroy(BloomFilter *bf) {
    if (bf) {
        free(bf->bits);
        free(bf);
    }
}
