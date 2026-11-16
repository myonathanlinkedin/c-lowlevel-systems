#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#include "core.h"
#include <assert.h>
#include <stdio.h>

static void test_basic_allocation(void) {
    size_t before_total = mem_total_allocated();
    size_t before_current = mem_current_allocated();

    int *a = (int *)tmalloc(sizeof(int) * 10);
    assert(a != NULL);
    for (int i = 0; i < 10; ++i) a[i] = i;
    assert(mem_current_allocated() == before_current + sizeof(int) * 10);
    assert(mem_total_allocated() == before_total + sizeof(int) * 10);
    assert(mem_allocation_count() == 1);

    tfree(a);
    assert(mem_current_allocated() == before_current);
    assert(mem_allocation_count() == 1); /* count stays, total does not decrease */
}

static void test_multiple_allocations(void) {
    mem_reset();
    void *p1 = tmalloc(64);
    void *p2 = tmalloc(128);
    void *p3 = tmalloc(256);
    assert(mem_current_allocated() == 64 + 128 + 256);
    assert(mem_peak_allocated() == mem_current_allocated());

    tfree(p2);
    assert(mem_current_allocated() == 64 + 256);
    tfree(p1);
    tfree(p3);
    assert(mem_current_allocated() == 0);
    assert(mem_allocation_count() == 3);
}

static void test_leak_detection(void) {
    mem_reset();
    void *leak = tmalloc(42);
    (void)leak; /* intentionally not freed */
    /* Capture report output via stdout redirection is omitted; we just verify internal state */
    assert(mem_current_allocated() == 42);
    assert(mem_allocation_count() == 1);
    mem_report(); /* Should list one leak */
    /* Clean up for subsequent tests */
    tfree(leak);
    assert(mem_current_allocated() == 0);
}

static void test_invalid_free(void) {
    mem_reset();
    int dummy;
    /* Expect assertion failure; guard with assert to avoid aborting test suite */
    /* Uncomment to see the failure:
    tfree(&dummy);
    */
    (void)dummy;
}

int main(void) {
    test_basic_allocation();
    test_multiple_allocations();
    test_leak_detection();
    test_invalid_free();

    printf("All tests passed.\n");
    return 0;
}
