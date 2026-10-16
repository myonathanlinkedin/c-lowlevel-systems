#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "core.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    list_t list;
    assert(list_init(&list));

    // Basic insertions
    assert(list_insert(&list, 10, 100));
    assert(list_insert(&list, 5,  50));
    assert(list_insert(&list, 20, 200));

    // Duplicate key updates existing value
    assert(list_insert(&list, 10, 101));

    // Successful finds
    int val = 0;
    assert(list_find(&list, 5,  &val) && val == 50);
    assert(list_find(&list, 10, &val) && val == 101);
    assert(list_find(&list, 20, &val) && val == 200);

    // Missing key
    assert(!list_find(&list, 15, &val));

    // Removals (head, middle, tail)
    assert(list_remove(&list, 5));   // head
    assert(!list_find(&list, 5, &val));

    assert(list_remove(&list, 10));  // now head
    assert(!list_find(&list, 10, &val));

    assert(list_remove(&list, 20));  // tail / only element
    assert(!list_find(&list, 20, &val));

    // List should be empty
    assert(list.head == NULL);

    // Stress test: bulk insert / find / remove
    for (int i = 0; i < 1000; ++i) {
        assert(list_insert(&list, i, i * 10));
    }
    for (int i = 0; i < 1000; ++i) {
        assert(list_find(&list, i, &val) && val == i * 10);
    }
    for (int i = 0; i < 1000; ++i) {
        assert(list_remove(&list, i));
    }
    assert(list.head == NULL);

    printf("All tests passed.\n");
    return 0;
}
