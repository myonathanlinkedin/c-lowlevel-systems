#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <assert.h>

BorrowedObject* mo_create(size_t size) {
    BorrowedObject* obj = (BorrowedObject*)malloc(sizeof(BorrowedObject));
    assert(obj != NULL);
    obj->data = (size > 0) ? malloc(size) : NULL;
    if (size > 0) {
        assert(obj->data != NULL);
    }
    obj->size = size;
    obj->immut_count = 0;
    obj->mut_active = false;
    return obj;
}

void mo_destroy(BorrowedObject* obj) {
    assert(obj != NULL);
    assert(obj->immut_count == 0);
    assert(!obj->mut_active);
    if (obj->data) {
        free(obj->data);
    }
    free(obj);
}

void* mo_borrow_immutable(BorrowedObject* obj) {
    assert(obj != NULL);
    assert(!obj->mut_active);
    obj->immut_count++;
    return obj->data;
}

void mo_release_immutable(BorrowedObject* obj) {
    assert(obj != NULL);
    assert(obj->immut_count > 0);
    obj->immut_count--;
}

void* mo_borrow_mutable(BorrowedObject* obj) {
    assert(obj != NULL);
    assert(!obj->mut_active);
    assert(obj->immut_count == 0);
    obj->mut_active = true;
    return obj->data;
}

void mo_release_mutable(BorrowedObject* obj) {
    assert(obj != NULL);
    assert(obj->mut_active);
    obj->mut_active = false;
}
