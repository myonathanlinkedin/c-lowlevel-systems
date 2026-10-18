#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

/*
 * Unit test driver for LSM-Tree MemTable and SSTable Flush Compaction Engine
 *
 * Tests:
 *  1. Basic put/get
 *  2. Overwrite existing key
 *  3. Delete (tombstone)
 *  4. Get after delete returns NULL
 *  5. Flush: memtable to SSTable
 *  6. Get from SSTable after flush
 *  7. Multiple flushes create multiple SSTables
 *  8. Compaction merges SSTables
 *  9. Get after compaction
 * 10. Delete in memtable shadows SSTable value
 * 11. Delete in SSTable shadows older SSTable
 * 12. Edge: empty engine get
 * 13. Edge: delete non-existent key
 * 14. Edge: flush empty memtable
 * 15. Stress: many writes trigger auto-flush
 */

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    do { \
        printf("  [TEST] %s ... ", name); \
    } while (0)

#define PASS() \
    do { \
        printf("PASS\n"); \
        tests_passed++; \
    } while (0)

#define FAIL(msg) \
    do { \
        printf("FAIL: %s\n", msg); \
        tests_failed++; \
    } while (0)

#define ASSERT_EQ_STR(a, b, msg) \
    do { \
        const char *_a = (a); \
        const char *_b = (b); \
        if ((_a == NULL && _b == NULL) || \
            (_a != NULL && _b != NULL && strcmp(_a, _b) == 0)) { \
            PASS(); \
        } else { \
            FAIL(msg); \
        } \
    } while (0)

#define ASSERT_TRUE(cond, msg) \
    do { \
        if (cond) { \
            PASS(); \
        } else { \
            FAIL(msg); \
        } \
    } while (0)

#define ASSERT_FALSE(cond, msg) \
    do { \
        if (!(cond)) { \
            PASS(); \
        } else { \
            FAIL(msg); \
        } \
    } while (0)

int main(void) {
    printf("=== LSM-Tree MemTable and SSTable Flush Compaction Engine ===\n\n");

    /* Test 1: Basic put/get */
    TEST("Basic put/get");
    {
        LSMEngine engine;
        lsm_init(&engine);
        lsm_put(&engine, "key1", "value1");
        const char *val = lsm_get(&engine, "key1");
        ASSERT_EQ_STR(val, "value1", "Expected value1");
        lsm_destroy(&engine);
    }

    /* Test 2: Overwrite existing key */
    TEST("Overwrite existing key");
    {
        LSMEngine engine;
        lsm_init(&engine);
        lsm_put(&engine, "key1", "value1");
        lsm_put(&engine, "key1", "value2");
        const char *val = lsm_get(&engine, "key1");
        ASSERT_EQ_STR(val, "value2", "Expected value2 after overwrite");
        lsm_destroy(&engine);
    }

    /* Test 3: Delete (tombstone) */
    TEST("Delete creates tombstone");
    {
        LSMEngine engine;
        lsm_init(&engine);
        lsm_put(&engine, "key1", "value1");
        lsm_delete(&engine, "key1");
        const char *val = lsm_get(&engine, "key1");
        ASSERT_TRUE(val == NULL, "Expected NULL after delete");
        lsm_destroy(&engine);
    }

    /* Test 4: Get after delete returns NULL */
    TEST("Get after delete returns NULL");
    {
        LSMEngine engine;
        lsm_init(&engine);
        lsm_put(&engine, "key1", "value1");
        lsm_delete(&engine, "key1");
        const char *val = lsm_get(&engine, "key1");
        ASSERT_TRUE(val == NULL, "Expected NULL");
        lsm_destroy(&engine);
    }

    /* Test 5: Flush memtable to SSTable */
    TEST("Flush memtable to SSTable");
    {
        LSMEngine engine;
        lsm_init(&engine);
        lsm_put(&engine, "key1", "value1");
        lsm_put(&engine, "key2", "value2");
        lsm_flush(&engine);
        ASSERT_TRUE(engine.sstable_count == 1, "Expected 1 SSTable after flush");
        ASSERT_TRUE(engine.memtable.count == 0, "Expected empty memtable after flush");
        lsm_destroy(&engine);
    }

    /* Test 6: Get from SSTable after flush */
    TEST("Get from SSTable after flush");
    {
        LSMEngine engine;
        lsm_init(&engine);
        lsm_put(&engine, "key1", "value1");
        lsm_put(&engine, "key2", "value2");
        lsm_flush(&engine);
        const char *val1 = lsm_get(&engine, "key1");
        const char *val2 = lsm_get(&engine, "key2");
        ASSERT_EQ_STR(val1, "value1", "Expected value1 from SSTable");
        ASSERT_EQ_STR(val2, "value2", "Expected value2 from SSTable");
        lsm_destroy(&engine);
    }

    /* Test 7: Multiple flushes create multiple SSTables */
    TEST("Multiple flushes create multiple SSTables");
    {
        LSMEngine engine;
        lsm_init(&engine);
        lsm_put(&engine, "key1", "value1");
        lsm_flush(&engine);
        lsm_put(&engine, "key2", "value2");
        lsm_flush(&engine);
        lsm_put(&engine, "key3", "value3");
        lsm_flush(&engine);
        ASSERT_TRUE(engine.sstable_count == 3, "Expected 3 SSTables");
        lsm_destroy(&engine);
    }

    /* Test 8: Compaction merges SSTables */
    TEST("Compaction merges SSTables");
    {
        LSMEngine engine;
        lsm_init(&engine);
        lsm_put(&engine, "key1", "value1");
        lsm_flush(&engine);
        lsm_put(&engine, "key2", "value2");
        lsm_flush(&engine);
        lsm_put(&engine, "key3", "value3");
        lsm_flush(&engine);
        lsm_compact(&engine);
        ASSERT_TRUE(engine.sstable_count == 1, "Expected 1 SSTable after compaction");
        ASSERT_TRUE(engine.sstables[0].count == 3, "Expected 3 keys in compacted SSTable");
        lsm_destroy(&engine);
    }

    /* Test 9: Get after compaction */
    TEST("Get after compaction");
    {
        LSMEngine engine;
        lsm_init(&engine);
        lsm_put(&engine, "key1", "value1");
        lsm_flush(&engine);
        lsm_put(&engine, "key2", "value2");
        lsm_flush(&engine);
        lsm_put(&engine, "key3", "value3");
        lsm_flush(&engine);
        lsm_compact(&engine);
        const char *val1 = lsm_get(&engine, "key1");
        const char *val2 = lsm_get(&engine, "key2");
        const char *val3 = lsm_get(&engine, "key3");
        ASSERT_EQ_STR(val1, "value1", "Expected value1");
        ASSERT_EQ_STR(val2, "value2", "Expected value2");
        ASSERT_EQ_STR(val3, "value3", "Expected value3");
        lsm_destroy(&engine);
    }

    /* Test 10: Delete in memtable shadows SSTable value */
    TEST("Delete in memtable shadows SSTable value");
    {
        LSMEngine engine;
        lsm_init(&engine);
        lsm_put(&engine, "key1", "value1");
        lsm_flush(&engine);
        lsm_delete(&engine, "key1");
        const char *val = lsm_get(&engine, "key1");
        ASSERT_TRUE(val == NULL, "Expected NULL (tombstone in memtable)");
        lsm_destroy(&engine);
    }

    /* Test 11: Delete in newer SSTable shadows older SSTable */
    TEST("Delete in newer SSTable shadows older SSTable");
    {
        LSMEngine engine;
        lsm_init(&engine);
        lsm_put(&engine, "key1", "value1");
        lsm_flush(&engine);
        lsm_delete(&engine, "key1");
        lsm_flush(&engine);
        const char *val = lsm_get(&engine, "key1");
        ASSERT_TRUE(val == NULL, "Expected NULL (tombstone in newer SSTable)");
        lsm_destroy(&engine);
    }

    /* Test 12: Edge - empty engine get */
    TEST("Edge: empty engine get");
    {
        LSMEngine engine;
        lsm_init(&engine);
        const char *val = lsm_get(&engine, "nonexistent");
        ASSERT_TRUE(val == NULL, "Expected NULL for non-existent key");
        lsm_destroy(&engine);
    }

    /* Test 13: Edge - delete non-existent key */
    TEST("Edge: delete non-existent key");
    {
        LSMEngine engine;
        lsm_init(&engine);
        lsm_delete(&engine, "nonexistent");
        const char *val = lsm_get(&engine, "nonexistent");
        ASSERT_TRUE(val == NULL, "Expected NULL");
        lsm_destroy(&engine);
    }

    /* Test 14: Edge - flush empty memtable */
    TEST("Edge: flush empty memtable");
    {
        LSMEngine engine;
        lsm_init(&engine);
        lsm_flush(&engine);
        ASSERT_TRUE(engine.sstable_count == 0, "Expected 0 SSTables");
        lsm_destroy(&engine);
    }

    /* Test 15: Stress - many writes trigger auto-flush */
    TEST("Stress: many writes trigger auto-flush");
    {
        LSMEngine engine;
        lsm_init(&engine);
        char key[32], val[64];
        for (int i = 0; i < 200; i++) {
            snprintf(key, sizeof(key), "key_%d", i);
            snprintf(val, sizeof(val), "value_%d", i);
            lsm_put(&engine, key, val);
        }
        /* Verify all keys are retrievable */
        bool all_found = true;
        for (int i = 0; i < 200; i++) {
            snprintf(key, sizeof(key), "key_%d", i);
            snprintf(val, sizeof(val), "value_%d", i);
            const char *got = lsm_get(&engine, key);
            if (got == NULL || strcmp(got, val) != 0) {
                all_found = false;
                break;
            }
        }
        ASSERT_TRUE(all_found, "All 200 keys should be retrievable");
        ASSERT_TRUE(engine.total_flushes > 0, "Auto-flush should have occurred");
        lsm_destroy(&engine);
    }

    /* Test 16: Overwrite after flush - new value in memtable shadows SSTable */
    TEST("Overwrite after flush - memtable shadows SSTable");
    {
        LSMEngine engine;
        lsm_init(&engine);
        lsm_put(&engine, "key1", "old_value");
        lsm_flush(&engine);
        lsm_put(&engine, "key1", "new_value");
        const char *val = lsm_get(&engine, "key1");
        ASSERT_EQ_STR(val, "new_value", "Expected new_value from memtable");
        lsm_destroy(&engine);
    }

    /* Test 17: Compaction preserves tombstones */
    TEST("Compaction preserves tombstones");
    {
        LSMEngine engine;
        lsm_init(&engine);
        lsm_put(&engine, "key1", "value1");
        lsm_put(&engine, "key2", "value2");
        lsm_flush(&engine);
        lsm_delete(&engine, "key1");
        lsm_flush(&engine);
        lsm_compact(&engine);
        const char *val1 = lsm_get(&engine, "key1");
        const char *val2 = lsm_get(&engine, "key2");
        ASSERT_TRUE(val1 == NULL, "Expected NULL for deleted key1");
        ASSERT_EQ_STR(val2, "value2", "Expected value2 for key2");
        lsm_destroy(&engine);
    }

    /* Test 18: Multiple compactions are idempotent */
    TEST("Multiple compactions are idempotent");
    {
        LSMEngine engine;
        lsm_init(&engine);
        lsm_put(&engine, "key1", "value1");
        lsm_flush(&engine);
        lsm_put(&engine, "key2", "value2");
        lsm_flush(&engine);
        lsm_compact(&engine);
        lsm_compact(&engine);
        ASSERT_TRUE(engine.sstable_count == 1, "Expected 1 SSTable");
        const char *val1 = lsm_get(&engine, "key1");
        const char *val2 = lsm_get(&engine, "key2");
        ASSERT_EQ_STR(val1, "value1", "Expected value1");
        ASSERT_EQ_STR(val2, "value2", "Expected value2");
        lsm_destroy(&engine);
    }

    /* Test 19: Sorted order in SSTable after flush */
    TEST("SSTable entries are sorted after flush");
    {
        LSMEngine engine;
        lsm_init(&engine);
        lsm_put(&engine, "zebra", "z_val");
        lsm_put(&engine, "apple", "a_val");
        lsm_put(&engine, "mango", "m_val");
        lsm_flush(&engine);
        const SSTable *st = &engine.sstables[0];
        bool sorted = true;
        for (size_t i = 1; i < st->count; i++) {
            if (strcmp(st->entries[i - 1].key, st->entries[i].key) > 0) {
                sorted = false;
                break;
            }
        }
        ASSERT_TRUE(sorted, "SSTable entries should be sorted");
        lsm_destroy(&engine);
    }

    /* Test 20: Binary search correctness in SSTable */
    TEST("Binary search correctness in SSTable");
    {
        LSMEngine engine;
        lsm_init(&engine);
        for (int i = 0; i < 50; i++) {
            char key[32], val[64];
            snprintf(key, sizeof(key), "key_%03d", i);
            snprintf(val, sizeof(val), "value_%03d", i);
            lsm_put(&engine, key, val);
        }
        lsm_flush(&engine);
        bool all_found = true;
        for (int i = 0; i < 50; i++) {
            char key[32], val[64];
            snprintf(key, sizeof(key), "key_%03d", i);
            snprintf(val, sizeof(val), "value_%03d", i);
            const char *got = lsm_get(&engine, key);
            if (got == NULL || strcmp(got, val) != 0) {
                all_found = false;
                break;
            }
        }
        ASSERT_TRUE(all_found, "All 50 keys found via binary search");
        lsm_destroy(&engine);
    }

    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("  Passed: %d\n", tests_passed);
    printf("  Failed: %d\n", tests_failed);
    printf("  Total:  %d\n", tests_passed + tests_failed);

    if (tests_failed == 0) {
        printf("\n  ALL TESTS PASSED\n");
        return 0;
    } else {
        printf("\n  SOME TESTS FAILED\n");
        return 1;
    }
}
