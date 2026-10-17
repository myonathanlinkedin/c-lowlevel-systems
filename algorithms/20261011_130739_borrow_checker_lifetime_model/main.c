#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "core.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void test_basic_flow(void) {
    BorrowedObject* obj = mo_create(10);
    /* Immutable borrows */
    void* p1 = mo_borrow_immutable(obj);
    void* p2 = mo_borrow_immutable(obj);
    assert(p1 == p2);
    memset(p1, 'A', obj->size);
    mo_release_immutable(obj);
    mo_release_immutable(obj);
    /* Mutable borrow */
    void* pm = mo_borrow_mutable(obj);
    for (size_t i = 0; i < obj->size; ++i) {
        ((char*)pm)[i] = (char)('a' + i);
    }
    mo_release_mutable(obj);
    /* Final immutable read */
    void* p3 = mo_borrow_immutable(obj);
    for (size_t i = 0; i < obj->size; ++i) {
        assert(((char*)p3)[i] == (char)('a' + i));
    }
    mo_release_immutable(obj);
    mo_destroy(obj);
}

static void test_edge_cases(void) {
    /* Zero-size allocation */
    BorrowedObject* obj = mo_create(0);
    assert(obj->data == NULL);
    void* pm = mo_borrow_mutable(obj);
    (void)pm;
    mo_release_mutable(obj);
    mo_destroy(obj);
}

int main(void) {
    test_basic_flow();
    test_edge_cases();
    printf("All tests passed.\n");
    return 0;
}
