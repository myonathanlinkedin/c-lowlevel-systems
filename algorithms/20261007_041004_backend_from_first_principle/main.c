#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "types.h"
#include "core.h"
#include <stdio.h>
#include <assert.h>

static void test_basic_operations(void) {
    Backend *db = backend_create(4);
    assert(db != NULL);
    assert(backend_set(db, "apple", 10));
    assert(backend_set(db, "banana", 20));
    assert(backend_set(db, "cherry", 30));

    int v = 0;
    assert(backend_get(db, "apple", &v) && v == 10);
    assert(backend_get(db, "banana", &v) && v == 20);
    assert(backend_get(db, "cherry", &v) && v == 30);
    assert(!backend_get(db, "date", &v));

    // overwrite existing
    assert(backend_set(db, "banana", 25));
    assert(backend_get(db, "banana", &v) && v == 25);

    // delete
    assert(backend_delete(db, "apple"));
    assert(!backend_get(db, "apple", &v));
    // delete non‑existent
    assert(!backend_delete(db, "fig"));

    backend_destroy(db);
}

static void test_collision_and_resize(void) {
    Backend *db = backend_create(2); // force early resize
    const char *keys[] = {"key1","key2","key3","key4","key5","key6","key7","key8"};
    for (size_t i = 0; i < 8; ++i) {
        assert(backend_set(db, keys[i], (int)i));
    }
    // verify all
    for (size_t i = 0; i < 8; ++i) {
        int v = -1;
        assert(backend_get(db, keys[i], &v) && v == (int)i);
    }
    // delete a few
    assert(backend_delete(db, "key3"));
    assert(backend_delete(db, "key6"));
    int dummy;
    assert(!backend_get(db, "key3", &dummy));
    assert(!backend_get(db, "key6", &dummy));

    // re‑insert after deletions
    assert(backend_set(db, "newkey", 99));
    assert(backend_get(db, "newkey", &dummy) && dummy == 99);

    backend_destroy(db);
}

int main(void) {
    test_basic_operations();
    test_collision_and_resize();
    printf("All backend tests passed.\n");
    return 0;
}
