#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "types.h"
#include "core.h"
#include <stdio.h>
#include <assert.h>

static void test_basic_operations(void) {
    List *list = list_create();
    assert(list != NULL);

    /* Insert a range of keys */
    for (int i = 1; i <= 10; ++i) {
        assert(list_insert(list, i, i * 100));
    }

    /* Duplicate insert must fail */
    assert(!list_insert(list, 5, 999));

    /* Search existing keys */
    for (int i = 1; i <= 10; ++i) {
        int val = 0;
        assert(list_search(list, i, &val));
        assert(val == i * 100);
    }

    /* Search missing key */
    int dummy;
    assert(!list_search(list, 42, &dummy));

    /* Delete a few keys */
    assert(list_delete(list, 3));
    assert(list_delete(list, 7));
    assert(!list_delete(list, 7));   // already removed

    /* Verify deletions */
    assert(!list_search(list, 3, NULL));
    assert(!list_search(list, 7, NULL));

    /* Remaining keys still accessible */
    for (int i = 1; i <= 10; ++i) {
        if (i == 3 || i == 7) continue;
        int val = 0;
        assert(list_search(list, i, &val));
        assert(val == i * 100);
    }

    /* Re‑insert a deleted key */
    assert(list_insert(list, 7, 777));
    int val = 0;
    assert(list_search(list, 7, &val));
    assert(val == 777);

    list_destroy(list);
}

static void test_edge_cases(void) {
    List *list = list_create();
    assert(list != NULL);

    /* Insert minimum and maximum int values */
    assert(list_insert(list, INT_MIN + 1, -1));
    assert(list_insert(list, INT_MAX, 1));

    int v;
    assert(list_search(list, INT_MIN + 1, &v));
    assert(v == -1);
    assert(list_search(list, INT_MAX, &v));
    assert(v == 1);

    /* Delete them */
    assert(list_delete(list, INT_MIN + 1));
    assert(list_delete(list, INT_MAX));
    assert(!list_search(list, INT_MIN + 1, NULL));
    assert(!list_search(list, INT_MAX, NULL));

    list_destroy(list);
}

int main(void) {
    test_basic_operations();
    test_edge_cases();
    printf("All tests passed.\n");
    return 0;
}
