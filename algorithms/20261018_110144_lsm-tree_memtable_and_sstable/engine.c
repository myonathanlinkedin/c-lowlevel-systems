#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Hash function: FNV-1a 32-bit
 * O(key_len) time, O(1) space
 */
static uint32_t fnv1a_hash(const char *str) {
    uint32_t hash = 2166136261u;
    while (*str) {
        hash ^= (uint8_t)*str++;
        hash *= 16777619u;
    }
    return hash;
}

/*
 * MemTable: open-addressing hash table with linear probing
 * Insert: O(1) amortized, O(capacity) worst case
 * Lookup: O(1) amortized, O(capacity) worst case
 * Space: O(capacity)
 */
void memtable_init(MemTable *mt) {
    memset(mt, 0, sizeof(MemTable));
    mt->count = 0;
}

bool memtable_is_full(const MemTable *mt) {
    return mt->count >= (size_t)(MEMTABLE_CAPACITY * 0.75);
}

size_t memtable_count(const MemTable *mt) {
    return mt->count;
}

static size_t memtable_probe(const MemTable *mt, const char *key) {
    uint32_t h = fnv1a_hash(key);
    size_t idx = (size_t)(h & (MEMTABLE_CAPACITY - 1));
    for (size_t i = 0; i < MEMTABLE_CAPACITY; i++) {
        size_t pos = (idx + i) & (MEMTABLE_CAPACITY - 1);
        if (!mt->used[pos]) {
            return pos;
        }
        if (strcmp(mt->entries[pos].key, key) == 0) {
            return pos;
        }
    }
    return MEMTABLE_CAPACITY; /* not found */
}

void memtable_put(MemTable *mt, const char *key, const char *value, bool deleted) {
    if (memtable_is_full(mt)) {
        /* Caller must flush before this point; assert in debug */
        fprintf(stderr, "ERROR: MemTable full, flush required\n");
        return;
    }
    size_t pos = memtable_probe(mt, key);
    if (pos >= MEMTABLE_CAPACITY) {
        fprintf(stderr, "ERROR: MemTable probe failed\n");
        return;
    }
    if (!mt->used[pos]) {
        mt->count++;
        mt->used[pos] = true;
    }
    strncpy(mt->entries[pos].key, key, MAX_KEY_LEN - 1);
    mt->entries[pos].key[MAX_KEY_LEN - 1] = '\0';
    if (deleted) {
        mt->entries[pos].deleted = true;
        mt->entries[pos].value[0] = '\0';
    } else {
        mt->entries[pos].deleted = false;
        strncpy(mt->entries[pos].value, value, MAX_VAL_LEN - 1);
        mt->entries[pos].value[MAX_VAL_LEN - 1] = '\0';
    }
}

const KVPair *memtable_get(const MemTable *mt, const char *key) {
    size_t pos = memtable_probe(mt, key);
    if (pos >= MEMTABLE_CAPACITY || !mt->used[pos]) {
        return NULL;
    }
    return &mt->entries[pos];
}

/*
 * SSTable: sorted array with binary search
 * Insert: O(n) due to memmove, O(log n) search
 * Lookup: O(log n)
 * Space: O(n)
 */
void sstable_init(SSTable *st, uint64_t seq) {
    memset(st, 0, sizeof(SSTable));
    st->count = 0;
    st->seq = seq;
}

bool sstable_is_full(const SSTable *st) {
    return st->count >= MAX_KEYS_PER_SSTABLE;
}

static int kv_compare(const void *a, const void *b) {
    const KVPair *ka = (const KVPair *)a;
    const KVPair *kb = (const KVPair *)b;
    return strcmp(ka->key, kb->key);
}

void sstable_insert_sorted(SSTable *st, const KVPair *kv) {
    if (sstable_is_full(st)) {
        fprintf(stderr, "ERROR: SSTable full\n");
        return;
    }
    /* Binary search for insertion point */
    size_t lo = 0, hi = st->count;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (strcmp(st->entries[mid].key, kv->key) < 0) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    /* lo is the insertion point */
    if (lo < st->count && strcmp(st->entries[lo].key, kv->key) == 0) {
        /* Update existing key */
        st->entries[lo] = *kv;
        return;
    }
    /* Shift elements right */
    if (lo < st->count) {
        memmove(&st->entries[lo + 1], &st->entries[lo],
                (st->count - lo) * sizeof(KVPair));
    }
    st->entries[lo] = *kv;
    st->count++;
}

const KVPair *sstable_get(const SSTable *st, const char *key) {
    size_t lo = 0, hi = st->count;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        int cmp = strcmp(st->entries[mid].key, key);
        if (cmp < 0) {
            lo = mid + 1;
        } else if (cmp > 0) {
            hi = mid;
        } else {
            return &st->entries[mid];
        }
    }
    return NULL;
}

/*
 * Merge two SSTables into dst (dst must be empty or will be overwritten)
 * O(n + m) time, O(n + m) space
 */
void sstable_merge_into(SSTable *dst, const SSTable *src) {
    if (dst == src) return;
    size_t i = 0, j = 0, k = 0;
    while (i < dst->count && j < src->count) {
        int cmp = strcmp(dst->entries[i].key, src->entries[j].key);
        if (cmp <= 0) {
            dst->entries[k++] = dst->entries[i++];
        } else {
            dst->entries[k++] = src->entries[j++];
        }
    }
    while (i < dst->count) {
        dst->entries[k++] = dst->entries[i++];
    }
    while (j < src->count) {
        dst->entries[k++] = src->entries[j++];
    }
    dst->count = k;
}

/*
 * LSM-Tree Engine
 */
void lsm_init(LSMEngine *engine) {
    memset(engine, 0, sizeof(LSMEngine));
    memtable_init(&engine->memtable);
    engine->next_seq = 1;
    engine->total_writes = 0;
    engine->total_flushes = 0;
    engine->total_compactions = 0;
}

void lsm_destroy(LSMEngine *engine) {
    (void)engine;
    /* No dynamic allocation; nothing to free */
}

void lsm_put(LSMEngine *engine, const char *key, const char *value) {
    if (memtable_is_full(&engine->memtable)) {
        lsm_flush(engine);
    }
    memtable_put(&engine->memtable, key, value, false);
    engine->total_writes++;
}

void lsm_delete(LSMEngine *engine, const char *key) {
    if (memtable_is_full(&engine->memtable)) {
        lsm_flush(engine);
    }
    memtable_put(&engine->memtable, key, "", true);
    engine->total_writes++;
}

const char *lsm_get(LSMEngine *engine, const char *key) {
    /* Check MemTable first */
    const KVPair *kv = memtable_get(&engine->memtable, key);
    if (kv) {
        if (kv->deleted) {
            return NULL;
        }
        return kv->value;
    }
    /* Check SSTables in reverse creation order (newest first) */
    for (size_t i = engine->sstable_count; i > 0; i--) {
        const SSTable *st = &engine->sstables[i - 1];
        const KVPair *skv = sstable_get(st, key);
        if (skv) {
            if (skv->deleted) {
                return NULL;
            }
            return skv->value;
        }
    }
    return NULL;
}

void lsm_flush(LSMEngine *engine) {
    if (engine->memtable.count == 0) {
        return;
    }
    if (engine->sstable_count >= MAX_SSTABLES) {
        fprintf(stderr, "ERROR: Max SSTables reached, compact first\n");
        return;
    }
    /* Create new SSTable from MemTable */
    SSTable *st = &engine->sstables[engine->sstable_count];
    sstable_init(st, engine->next_seq++);

    /* Copy all entries from MemTable to SSTable */
    for (size_t i = 0; i < MEMTABLE_CAPACITY; i++) {
        if (engine->memtable.used[i]) {
            sstable_insert_sorted(st, &engine->memtable.entries[i]);
        }
    }
    engine->sstable_count++;
    engine->total_flushes++;

    /* Reset MemTable */
    memtable_init(&engine->memtable);
}

void lsm_compact(LSMEngine *engine) {
    if (engine->sstable_count <= 1) {
        return;
    }
    /* Merge all SSTables into the first one */
    SSTable *dst = &engine->sstables[0];
    for (size_t i = 1; i < engine->sstable_count; i++) {
        /* Merge sstables[i] into dst */
        SSTable tmp;
        sstable_init(&tmp, 0);
        /* Copy dst into tmp */
        tmp.count = dst->count;
        memcpy(tmp.entries, dst->entries, dst->count * sizeof(KVPair));
        /* Merge tmp and sstables[i] into dst */
        size_t a = 0, b = 0, k = 0;
        while (a < tmp.count && b < engine->sstables[i].count) {
            int cmp = strcmp(tmp.entries[a].key, engine->sstables[i].entries[b].key);
            if (cmp <= 0) {
                dst->entries[k++] = tmp.entries[a++];
            } else {
                dst->entries[k++] = engine->sstables[i].entries[b++];
            }
        }
        while (a < tmp.count) {
            dst->entries[k++] = tmp.entries[a++];
        }
        while (b < engine->sstables[i].count) {
            dst->entries[k++] = engine->sstables[i].entries[b++];
        }
        dst->count = k;
    }
    engine->sstable_count = 1;
    engine->total_compactions++;
}
