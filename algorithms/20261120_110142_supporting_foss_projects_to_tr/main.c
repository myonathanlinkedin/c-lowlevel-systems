#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "core.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main(void) {
    /* Test creation */
    rc_string *s1 = rc_string_new("hello");
    assert(s1 != NULL);
    assert(strcmp(rc_string_get(s1), "hello") == 0);
    assert(rc_string_refcount(s1) == 1);
    assert(rc_string_alloc_count() == 1);

    /* Test retain */
    rc_string_retain(s1);
    assert(rc_string_refcount(s1) == 2);

    /* Test release */
    rc_string_release(s1);
    assert(rc_string_refcount(s1) == 1);
    rc_string_release(s1);
    assert(rc_string_alloc_count() == 0);

    /* Test empty string */
    rc_string *s2 = rc_string_new("");
    assert(s2 != NULL);
    assert(strcmp(rc_string_get(s2), "") == 0);
    rc_string_release(s2);
    assert(rc_string_alloc_count() == 0);

    /* Test null init */
    rc_string *s3 = rc_string_new(NULL);
    assert(s3 != NULL);
    assert(strcmp(rc_string_get(s3), "") == 0);
    rc_string_release(s3);
    assert(rc_string_alloc_count() == 0);

    /* Test multiple retains and releases */
    rc_string *s4 = rc_string_new("test");
    rc_string_retain(s4);
    rc_string_retain(s4);
    assert(rc_string_refcount(s4) == 3);
    rc_string_release(s4);
    assert(rc_string_refcount(s4) == 2);
    rc_string_release(s4);
    assert(rc_string_refcount(s4) == 1);
    rc_string_release(s4);
    assert(rc_string_alloc_count() == 0);

    printf("All tests passed.\n");
    return 0;
}
