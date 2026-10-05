#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct {
    int   refcount;
    size_t capacity;
    int   data[];
} Buffer;

typedef struct {
    Buffer *buf;
    size_t  size;
} CowVector;

/* Internal: allocate a new buffer */
static Buffer *buffer_new(size_t cap) {
    Buffer *b = (Buffer *)malloc(sizeof(Buffer) + cap * sizeof(int));
    if (!b) abort();
    b->refcount = 1;
    b->capacity = cap;
    return b;
}

/* Internal: increase reference count */
static void buffer_inc(Buffer *b) {
    if (b) ++b->refcount;
}

/* Internal: decrease reference count and free if zero */
static void buffer_dec(Buffer *b) {
    if (b && --b->refcount == 0) free(b);
}

/* Ensure the vector has a unique buffer (copy-on-write) */
static void cowvec_ensure_unique(CowVector *v) {
    if (v->buf && v->buf->refcount > 1) {
        Buffer *old = v->buf;
        Buffer *newb = buffer_new(old->capacity);
        memcpy(newb->data, old->data, v->size * sizeof(int));
        v->buf = newb;
        buffer_dec(old);
    }
}

/* Public API */
static void cowvec_init(CowVector *v) {
    v->buf = NULL;
    v->size = 0;
}

static void cowvec_destroy(CowVector *v) {
    buffer_dec(v->buf);
    v->buf = NULL;
    v->size = 0;
}

static void cowvec_reserve(CowVector *v, size_t new_cap) {
    if (!v->buf) {
        v->buf = buffer_new(new_cap);
        return;
    }
    if (new_cap <= v->buf->capacity) return;
    cowvec_ensure_unique(v);
    Buffer *old = v->buf;
    Buffer *nb = (Buffer *)realloc(old, sizeof(Buffer) + new_cap * sizeof(int));
    if (!nb) abort();
    nb->capacity = new_cap;
    v->buf = nb;
}

static void cowvec_push(CowVector *v, int value) {
    if (!v->buf || v->size == v->buf->capacity) {
        size_t new_cap = v->buf ? v->buf->capacity * 2 : 4;
        if (new_cap < 4) new_cap = 4;
        cowvec_reserve(v, new_cap);
    }
    cowvec_ensure_unique(v);
    v->buf->data[v->size++] = value;
}

static int cowvec_get(const CowVector *v, size_t idx) {
    assert(v->buf && idx < v->size);
    return v->buf->data[idx];
}

static void cowvec_set(CowVector *v, size_t idx, int value) {
    assert(v->buf && idx < v->size);
    cowvec_ensure_unique(v);
    v->buf->data[idx] = value;
}

static size_t cowvec_size(const CowVector *v) { return v->size; }
static size_t cowvec_capacity(const CowVector *v) { return v->buf ? v->buf->capacity : 0; }

static void cowvec_clone(const CowVector *src, CowVector *dest) {
    dest->buf = src->buf;
    dest->size = src->size;
    buffer_inc(dest->buf);
}

/* Unit tests */
int main(void) {
    CowVector a, b, c;
    cowvec_init(&a);
    for (int i = 0; i < 10; ++i) cowvec_push(&a, i * 2);
    assert(cowvec_size(&a) == 10);
    assert(cowvec_get(&a, 5) == 10);

    cowvec_clone(&a, &b);
    assert(cowvec_size(&b) == 10);
    assert(cowvec_get(&b, 5) == 10);
    assert(b.buf->refcount == 2);

    cowvec_set(&b, 5, 999);
    assert(cowvec_get(&b, 5) == 999);
    assert(cowvec_get(&a, 5) == 10);
    assert(b.buf->refcount == 1);
    assert(a.buf->refcount == 1);

    cowvec_push(&b, 1234);
    assert(cowvec_size(&b) == 11);
    assert(cowvec_get(&b, 10) == 1234);

    cowvec_clone(&b, &c);
    assert(cowvec_size(&c) == 11);
    assert(cowvec_get(&c, 10) == 1234);
    assert(c.buf->refcount == 2);

    cowvec_reserve(&c, 50);
    assert(cowvec_capacity(&c) >= 50);
    assert(c.buf->refcount == 1); /* reserve forces uniqueness */

    cowvec_destroy(&a);
    cowvec_destroy(&b);
    cowvec_destroy(&c);
    return 0;
}
