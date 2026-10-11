#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct ManagedObject {
    void *data;                 // raw memory block
    size_t size;                // size of the block
    size_t shared_count;        // number of active shared borrows
    bool mut_borrowed;          // true if a mutable borrow is active
};

/* Allocate a new managed object. */
ManagedObject* mo_create(size_t size) {
    ManagedObject *mo = (ManagedObject*)malloc(sizeof(ManagedObject));
    if (!mo) return NULL;
    mo->data = malloc(size);
    if (!mo->data) {
        free(mo);
        return NULL;
    }
    mo->size = size;
    mo->shared_count = 0;
    mo->mut_borrowed = false;
    /* Zero‑initialize the payload for deterministic tests. */
    memset(mo->data, 0, size);
    return mo;
}

/* Destroy a managed object. Must be unborrowed. */
void mo_destroy(ManagedObject* mo) {
    assert(mo != NULL);
    assert(mo->shared_count == 0);
    assert(!mo->mut_borrowed);
    free(mo->data);
    free(mo);
}

/* Acquire a shared (immutable) borrow. */
const void* mo_borrow_shared(ManagedObject* mo) {
    assert(mo != NULL);
    if (mo->mut_borrowed) {
        /* Cannot share while mutable borrow exists. */
        return NULL;
    }
    ++mo->shared_count;
    return mo->data;
}

/* Release a shared borrow. */
void mo_return_shared(ManagedObject* mo) {
    assert(mo != NULL);
    assert(mo->shared_count > 0);
    --mo->shared_count;
}

/* Acquire an exclusive mutable borrow. */
void* mo_borrow_mut(ManagedObject* mo) {
    assert(mo != NULL);
    if (mo->mut_borrowed || mo->shared_count > 0) {
        /* Any existing borrow blocks mutable access. */
        return NULL;
    }
    mo->mut_borrowed = true;
    return mo->data;
}

/* Release a mutable borrow. */
void mo_return_mut(ManagedObject* mo) {
    assert(mo != NULL);
    assert(mo->mut_borrowed);
    mo->mut_borrowed = false;
}
