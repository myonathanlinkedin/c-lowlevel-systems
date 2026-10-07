#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>

#include "types.h"
#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#define LOAD_FACTOR_MAX 0.75

static uint64_t fnv1a_hash(const char *s) {
    uint64_t hash = 14695981039346656037ULL;
    while (*s) {
        hash ^= (unsigned char)*s++;
        hash *= 1099511628211ULL;
    }
    return hash;
}

/* Internal: find slot index for a key.
   Returns index where key is found, or where it can be inserted (empty or tombstone).
   If out_found is non-NULL, set to true if key already exists. */
static size_t find_slot(const Backend *db, const char *key, bool *out_found) {
    size_t mask = db->capacity - 1;
    uint64_t h = fnv1a_hash(key);
    size_t idx = (size_t)(h & mask);
    size_t first_tombstone = SIZE_MAX;
    while (true) {
        char *slot = db->keys[idx];
        if (slot == NULL) {
            if (first_tombstone != SIZE_MAX) {
                idx = first_tombstone;
            }
            if (out_found) *out_found = false;
            return idx;
        }
        if (slot == (char*)1) { // tombstone
            if (first_tombstone == SIZE_MAX) first_tombstone = idx;
        } else if (strcmp(slot, key) == 0) {
            if (out_found) *out_found = true;
            return idx;
        }
        idx = (idx + 1) & mask;
    }
}

/* Resize table when load factor exceeds threshold */
static bool backend_resize(Backend *db, size_t new_capacity) {
    char **old_keys = db->keys;
    int *old_vals = db->values;
    size_t old_cap = db->capacity;

    char **new_keys = calloc(new_capacity, sizeof(char*));
    int *new_vals = calloc(new_capacity, sizeof(int));
    if (!new_keys || !new_vals) {
        free(new_keys);
        free(new_vals);
        return false;
    }

    db->keys = new_keys;
    db->values = new_vals;
    db->capacity = new_capacity;
    db->count = 0;

    for (size_t i = 0; i < old_cap; ++i) {
        char *k = old_keys[i];
        if (k && k != (char*)1) {
            int v = old_vals[i];
            // re-insert
            bool dummy;
            size_t idx = find_slot(db, k, &dummy);
            db->keys[idx] = k;          // transfer ownership of string
            db->values[idx] = v;
            db->count++;
        } else {
            free(k); // free NULL or tombstone (no-op for tombstone)
        }
    }
    free(old_keys);
    free(old_vals);
    return true;
}

Backend* backend_create(size_t initial_capacity) {
    // capacity must be power of two for mask trick
    size_t cap = 8;
    while (cap < initial_capacity) cap <<= 1;
    Backend *db = calloc(1, sizeof(Backend));
    if (!db) return NULL;
    db->keys = calloc(cap, sizeof(char*));
    db->values = calloc(cap, sizeof(int));
    if (!db->keys || !db->values) {
        backend_destroy(db);
        return NULL;
    }
    db->capacity = cap;
    db->count = 0;
    return db;
}

void backend_destroy(Backend *db) {
    if (!db) return;
    if (db->keys) {
        for (size_t i = 0; i < db->capacity; ++i) {
            char *k = db->keys[i];
            if (k && k != (char*)1) free(k);
        }
        free(db->keys);
    }
    free(db->values);
    free(db);
}

bool backend_set(Backend *db, const char *key, int value) {
    if (!db || !key) return false;
    if ((double)(db->count + 1) / (double)db->capacity > LOAD_FACTOR_MAX) {
        if (!backend_resize(db, db->capacity * 2)) return false;
    }
    bool found;
    size_t idx = find_slot(db, key, &found);
    if (found) {
        db->values[idx] = value;
        return true;
    }
    // insert new
    char *copy = strdup(key);
    if (!copy) return false;
    db->keys[idx] = copy;
    db->values[idx] = value;
    db->count++;
    return true;
}

bool backend_get(const Backend *db, const char *key, int *out_value) {
    if (!db || !key) return false;
    bool found;
    size_t idx = find_slot(db, key, &found);
    if (found) {
        if (out_value) *out_value = db->values[idx];
        return true;
    }
    return false;
}

bool backend_delete(Backend *db, const char *key) {
    if (!db || !key) return false;
    bool found;
    size_t idx = find_slot(db, key, &found);
    if (!found) return false;
    free(db->keys[idx]);
    db->keys[idx] = (char*)1; // tombstone
    // value left untouched
    db->count--;
    return true;
}
