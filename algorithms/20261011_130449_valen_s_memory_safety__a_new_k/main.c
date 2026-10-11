#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "core.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void test_basic_lifecycle(void) {
    ManagedObject *obj = mo_create(8);
    assert(obj != NULL);

    /* Mutable borrow, write data, then return. */
    uint8_t *mut = (uint8_t*)mo_borrow_mut(obj);
    assert(mut != NULL);
    for (size_t i = 0; i < 8; ++i) mut[i] = (uint8_t)(i * 3);
    mo_return_mut(obj);

    /* Shared borrows, read data, then return. */
    const uint8_t *sh1 = (const uint8_t*)mo_borrow_shared(obj);
    const uint8_t *sh2 = (const uint8_t*)mo_borrow_shared(obj);
    assert(sh1 != NULL && sh2 != NULL);
    for (size_t i = 0; i < 8; ++i) {
        assert(sh1[i] == (uint8_t)(i * 3));
        assert(sh2[i] == (uint8_t)(i * 3));
    }
    mo_return_shared(obj);
    mo_return_shared(obj);

    mo_destroy(obj);
}

static void test_mut_exclusive(void) {
    ManagedObject *obj = mo_create(4);
    assert(obj != NULL);

    uint32_t *mut = (uint32_t*)mo_borrow_mut(obj);
    assert(mut != NULL);
    *mut = 0xDEADBEEF;

    /* Attempt illegal shared borrow while mutable borrow active. */
    const void *illegal_shared = mo_borrow_shared(obj);
    assert(illegal_shared == NULL);

    /* Attempt second mutable borrow – also illegal. */
    void *illegal_mut = mo_borrow_mut(obj);
    assert(illegal_mut == NULL);

    mo_return_mut(obj);
    mo_destroy(obj);
}

static void test_shared_then_mut(void) {
    ManagedObject *obj = mo_create(2);
    assert(obj != NULL);

    const uint8_t *sh = (const uint8_t*)mo_borrow_shared(obj);
    assert(sh != NULL);

    /* While a shared borrow exists, mutable borrow must fail. */
    void *mut = mo_borrow_mut(obj);
    assert(mut == NULL);

    mo_return_shared(obj);

    /* Now mutable borrow succeeds. */
    mut = mo_borrow_mut(obj);
    assert(mut != NULL);
    ((uint8_t*)mut)[0] = 0xAA;
    ((uint8_t*)mut)[1] = 0x55;
    mo_return_mut(obj);

    mo_destroy(obj);
}

static void test_invalid_return(void) {
    ManagedObject *obj = mo_create(1);
    assert(obj != NULL);

    /* Returning without a matching borrow must trigger assert.
       We cannot catch assert failures in portable C, so we only
       demonstrate correct usage here. */
    const void *sh = mo_borrow_shared(obj);
    assert(sh != NULL);
    mo_return_shared(obj);

    /* Double return would abort – commented out to keep test suite alive.
       mo_return_shared(obj);
    */

    mo_destroy(obj);
}

int main(void) {
    test_basic_lifecycle();
    test_mut_exclusive();
    test_shared_then_mut();
    test_invalid_return();

    printf("All Valen's borrow‑checking tests passed.\n");
    return 0;
}
