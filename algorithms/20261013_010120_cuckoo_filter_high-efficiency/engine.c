#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Internal representation */
struct CuckooFilter {
    size_t bucket_count;      /* Number of buckets (must be power of two for fast modulo) */
    size_t bucket_size;       /* Slots per bucket */
    uint8_t fingerprint_mask;/* Mask to keep only the requested bits (<=8) */
    uint8_t *buckets;         /* Flat array: bucket_count * bucket_size */
};

/* ---------- Helper hash functions (FNV-1a 64‑bit) ---------- */
static uint64_t fnv1a_hash(const void *data, size_t len)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint64_t hash = 14695981039346656037ULL;
    for (size_t i = 0; i < len; ++i) {
        hash ^= (uint64_t)bytes[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

/* Produce a non‑zero fingerprint limited to `bits` (1‑8) */
static uint8_t fingerprint(const void *item, size_t len, uint8_t bits)
{
    uint64_t h = fnv1a_hash(item, len);
    uint8_t fp = (uint8_t)(h & ((1ULL << bits) - 1ULL));
    /* Zero fingerprint would be indistinguishable from an empty slot */
    if (fp == 0) {
        fp = 1;
    }
    return fp;
}

/* Primary bucket index */
static size_t index_hash(uint64_t hash, size_t bucket_count)
{
    return (size_t)(hash & (bucket_count - 1ULL)); /* bucket_count must be power of two */
}

/* Alternate bucket index given a fingerprint */
static size_t alt_index(size_t index, uint8_t fp, size_t bucket_count)
{
    uint64_t h = fnv1a_hash(&fp, sizeof(fp));
    return (index ^ (size_t)h) & (bucket_count - 1ULL);
}

/* Access a bucket's slot */
static uint8_t *bucket_slot(CuckooFilter *cf, size_t bucket_idx, size_t slot)
{
    return cf->buckets + (bucket_idx * cf->bucket_size) + slot;
}

/* ---------- Public API implementation ---------- */
CuckooFilter *cf_create(size_t bucket_count,
                        size_t bucket_size,
                        uint8_t fingerprint_bits)
{
    /* bucket_count must be a power of two for modulo optimisation */
    if (bucket_count == 0 || (bucket_count & (bucket_count - 1U)) != 0U) {
        return NULL;
    }
    if (bucket_size == 0 || fingerprint_bits == 0 || fingerprint_bits > 8U) {
        return NULL;
    }

    CuckooFilter *cf = (CuckooFilter *)malloc(sizeof(CuckooFilter));
    if (!cf) {
        return NULL;
    }
    cf->bucket_count = bucket_count;
    cf->bucket_size = bucket_size;
    cf->fingerprint_mask = (uint8_t)((1U << fingerprint_bits) - 1U);
    cf->buckets = (uint8_t *)calloc(bucket_count * bucket_size, sizeof(uint8_t));
    if (!cf->buckets) {
        free(cf);
        return NULL;
    }
    return cf;
}

void cf_destroy(CuckooFilter *cf)
{
    if (cf) {
        free(cf->buckets);
        free(cf);
    }
}

/* Insert a fingerprint, performing evictions if necessary */
bool cf_insert(CuckooFilter *cf,
               const void *item,
               size_t len)
{
    if (!cf || !item) {
        return false;
    }

    uint8_t fp = fingerprint(item, len, 8U) & cf->fingerprint_mask;
    uint64_t h = fnv1a_hash(item, len);
    size_t i1 = index_hash(h, cf->bucket_count);
    size_t i2 = alt_index(i1, fp, cf->bucket_count);

    /* Try to place in first bucket */
    for (size_t s = 0; s < cf->bucket_size; ++s) {
        uint8_t *slot = bucket_slot(cf, i1, s);
        if (*slot == 0) {
            *slot = fp;
            return true;
        }
    }
    /* Try second bucket */
    for (size_t s = 0; s < cf->bucket_size; ++s) {
        uint8_t *slot = bucket_slot(cf, i2, s);
        if (*slot == 0) {
            *slot = fp;
            return true;
        }
    }

    /* Need to relocate */
    size_t cur_idx = i1;
    uint8_t cur_fp = fp;

    for (uint32_t n = 0; n < CF_MAX_RELOCATIONS; ++n) {
        /* Choose a random slot in the current bucket (deterministic round‑robin) */
        size_t slot = n % cf->bucket_size;
        uint8_t *victim = bucket_slot(cf, cur_idx, slot);
        uint8_t evicted_fp = *victim;
        *victim = cur_fp;
        cur_fp = evicted_fp;
        cur_idx = alt_index(cur_idx, cur_fp, cf->bucket_count);

        /* Try to place evicted fingerprint in its alternate bucket */
        for (size_t s = 0; s < cf->bucket_size; ++s) {
            uint8_t *alt_slot = bucket_slot(cf, cur_idx, s);
            if (*alt_slot == 0) {
                *alt_slot = cur_fp;
                return true;
            }
        }
    }
    /* Insertion failed after too many relocations */
    return false;
}

/* Check presence of an item */
bool cf_contains(const CuckooFilter *cf,
                 const void *item,
                 size_t len)
{
    if (!cf || !item) {
        return false;
    }

    uint8_t fp = fingerprint(item, len, 8U) & cf->fingerprint_mask;
    uint64_t h = fnv1a_hash(item, len);
    size_t i1 = index_hash(h, cf->bucket_count);
    size_t i2 = alt_index(i1, fp, cf->bucket_count);

    for (size_t s = 0; s < cf->bucket_size; ++s) {
        if (bucket_slot((CuckooFilter *)cf, i1, s)[0] == fp ||
            bucket_slot((CuckooFilter *)cf, i2, s)[0] == fp) {
            return true;
        }
    }
    return false;
}

/* Delete an item if present */
bool cf_delete(CuckooFilter *cf,
               const void *item,
               size_t len)
{
    if (!cf || !item) {
        return false;
    }

    uint8_t fp = fingerprint(item, len, 8U) & cf->fingerprint_mask;
    uint64_t h = fnv1a_hash(item, len);
    size_t i1 = index_hash(h, cf->bucket_count);
    size_t i2 = alt_index(i1, fp, cf->bucket_count);

    /* Helper to remove from a bucket by shifting */
    for (size_t bucket = 0; bucket < 2; ++bucket) {
        size_t idx = (bucket == 0) ? i1 : i2;
        for (size_t s = 0; s < cf->bucket_size; ++s) {
            uint8_t *slot = bucket_slot(cf, idx, s);
            if (*slot == fp) {
                /* Shift remaining entries left */
                for (size_t t = s; t + 1 < cf->bucket_size; ++t) {
                    bucket_slot(cf, idx, t)[0] = bucket_slot(cf, idx, t + 1)[0];
                }
                bucket_slot(cf, idx, cf->bucket_size - 1)[0] = 0;
                return true;
            }
        }
    }
    return false;
}
