#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdatomic.h>

static atomic_uint alloc_counter = 0;

rc_string *rc_string_new(const char *init) {
    rc_string *s = (rc_string *)malloc(sizeof(rc_string));
    if (!s) return NULL;
    size_t len = init ? strlen(init) : 0;
    s->data = (char *)malloc(len + 1);
    if (!s->data) {
        free(s);
        return NULL;
    }
    if (init) memcpy(s->data, init, len + 1);
    else s->data[0] = '\0';
    atomic_init(&s->refcount, 1);
    atomic_fetch_add(&alloc_counter, 1);
    return s;
}

rc_string *rc_string_retain(rc_string *s) {
    assert(s);
    atomic_fetch_add(&s->refcount, 1);
    return s;
}

void rc_string_release(rc_string *s) {
    assert(s);
    unsigned int old = atomic_fetch_sub(&s->refcount, 1);
    if (old == 1) {
        free(s->data);
        free(s);
        atomic_fetch_sub(&alloc_counter, 1);
    }
}

const char *rc_string_get(const rc_string *s) {
    assert(s);
    return s->data;
}

size_t rc_string_refcount(const rc_string *s) {
    assert(s);
    return atomic_load(&s->refcount);
}

size_t rc_string_alloc_count(void) {
    return atomic_load(&alloc_counter);
}
