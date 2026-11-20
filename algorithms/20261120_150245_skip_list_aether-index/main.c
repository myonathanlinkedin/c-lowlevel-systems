#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#include "types.h"
#include <assert.h>
#include <stdio.h>

static void test_basic_operations(void) {
    SkipList *list = skiplist_create();
    assert(list != NULL);
    assert(skiplist_size(list) == 0);

    /* Insert elements */
    assert(skiplist_insert(list, 10, 100));
    assert(skiplist_insert(list, 20, 200));
    assert(skiplist_insert(list, 5, 50));
    assert(skiplist_size(list) == 3);

    /* Search existing keys */
    int val = 0;
    assert(skiplist_search(list, 10, &val) && val == 100);
    assert(skiplist_search(list, 20, &val) && val == 200);
    assert(skiplist_search(list, 5, &val) && val == 50);

    /* Search non‑existing key */
    assert(!skiplist_search(list, 15, &val));

    /* Update existing key */
    assert(skiplist_insert(list, 10, 101));
    assert(skiplist_search(list, 10, &val) && val == 101);
    assert(skiplist_size(list) == 3); /* size unchanged */

    /* Delete a middle element */
    assert(skiplist_delete(list, 10));
    assert(!skiplist_search(list, 10, &val));
    assert(skiplist_size(list) == 2);

    /* Delete non‑existing key */
    assert(!skiplist_delete(list, 999));

    /* Delete remaining elements */
    assert(skiplist_delete(list, 5));
    assert(skiplist_delete(list, 20));
    assert(skiplist_size(list) == 0);
    assert(!skiplist_search(list, 5, &val));

    skiplist_destroy(list);
}

static void test_edge_cases(void) {
    SkipList *list = skiplist_create();
    assert(list != NULL);

    /* Insert INT_MIN and INT_MAX */
    assert(skiplist_insert(list, INT32_MIN, -1));
    assert(skiplist_insert(list, INT32_MAX, 1));
    assert(skiplist_size(list) == 2);

    int v = 0;
    assert(skiplist_search(list, INT32_MIN, &v) && v == -1);
    assert(skiplist_search(list, INT32_MAX, &v) && v == 1);

    /* Delete INT_MIN */
    assert(skiplist_delete(list, INT32_MIN));
    assert(!skiplist_search(list, INT32_MIN, &v));
    assert(skiplist_size(list) == 1);

    /* Re‑insert many sequential keys to stress levels */
    for (int i = -1000; i <= 1000; ++i) {
        assert(skiplist_insert(list, i, i * 2));
    }
    assert(skiplist_size(list) == 2001);

    /* Verify a few random keys */
    assert(skiplist_search(list, -500, &v) && v == -1000);
    assert(skiplist_search(list, 0, &v) && v == 0);
    assert(skiplist_search(list, 999, &v) && v == 1998);

    /* Delete a range */
    for (int i = -500; i <= 500; ++i) {
        assert(skiplist_delete(list, i));
    }
    assert(skiplist_size(list) == 2001 - 1001);

    /* Ensure deleted keys are gone */
    assert(!skiplist_search(list, 0, &v));
    assert(skiplist_search(list, -501, &v));
    assert(skiplist_search(list, 501, &v));

    skiplist_destroy(list);
}

int main(void) {
    test_basic_operations();
    test_edge_cases();

    printf("All SkipList Aether‑Index tests passed.\n");
    return 0;
}
