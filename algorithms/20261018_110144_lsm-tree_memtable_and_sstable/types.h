#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*
 * LSM-Tree MemTable and SSTable Flush Compaction Engine
 *
 * Design invariants:
 *  - MemTable: fixed-capacity open-addressing hash table (linear probing).
 *    O(1) amortized insert/lookup. Load factor capped at 0.75.
 *  - SSTable: sorted key-value array with binary search. O(log n) lookup.
 *  - Compaction: k-way merge of SSTables into a single sorted SSTable.
 *    O(N log K) where N = total keys, K = number of tables.
 *  - Read path: check MemTable first, then SSTables in reverse creation order.
 *  - Write path: insert into MemTable; flush when capacity exceeded.
 *  - Delete: tombstone marker (value = NULL, deleted = true).
 */

#define MEMTABLE_CAPACITY 64
#define MAX_SSTABLES 16
#define MAX_KEYS_PER_SSTABLE 256
#define MAX_KEY_LEN 32
#define MAX_VAL_LEN 64

/* Tombstone marker */
#define TOMBSTONE ((const char *)0xDEADBEEF)

/* Key-value pair */
typedef struct {
    char key[MAX_KEY_LEN];
    char value[MAX_VAL_LEN];
    bool deleted; /* true if tombstone */
} KVPair;

/* MemTable: open-addressing hash table */
typedef struct {
    KVPair entries[MEMTABLE_CAPACITY];
    bool used[MEMTABLE_CAPACITY];
    size_t count;
} MemTable;

/* SSTable: sorted array of KV pairs */
typedef struct {
    KVPair entries[MAX_KEYS_PER_SSTABLE];
    size_t count;
    uint64_t seq; /* sequence number for ordering */
} SSTable;

/* LSM-Tree engine state */
typedef struct {
    MemTable memtable;
    SSTable sstables[MAX_SSTABLES];
    size_t sstable_count;
    uint64_t next_seq;
    uint64_t total_writes;
    uint64_t total_flushes;
    uint64_t total_compactions;
} LSMEngine;

/* API */
void lsm_init(LSMEngine *engine);
void lsm_put(LSMEngine *engine, const char *key, const char *value);
void lsm_delete(LSMEngine *engine, const char *key);
const char *lsm_get(LSMEngine *engine, const char *key);
void lsm_flush(LSMEngine *engine);
void lsm_compact(LSMEngine *engine);
void lsm_destroy(LSMEngine *engine);

/* MemTable helpers */
void memtable_init(MemTable *mt);
void memtable_put(MemTable *mt, const char *key, const char *value, bool deleted);
const KVPair *memtable_get(const MemTable *mt, const char *key);
bool memtable_is_full(const MemTable *mt);
size_t memtable_count(const MemTable *mt);

/* SSTable helpers */
void sstable_init(SSTable *st, uint64_t seq);
void sstable_insert_sorted(SSTable *st, const KVPair *kv);
const KVPair *sstable_get(const SSTable *st, const char *key);
bool sstable_is_full(const SSTable *st);

/* Compaction */
void sstable_merge_into(SSTable *dst, const SSTable *src);
