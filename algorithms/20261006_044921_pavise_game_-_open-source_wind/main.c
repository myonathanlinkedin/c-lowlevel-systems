#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "core.c" // Include core implementation directly for single-file build

static void test_add_and_suppress(void) {
    ProcessManager pm;
    pm_init(&pm);
    assert(pm_add(&pm, 1001));
    assert(pm_add(&pm, 1002));
    assert(!pm_add(&pm, 1001)); // duplicate
    assert(pm_suppress(&pm, 1001));
    assert(pm_is_suppressed(&pm, 1001));
    assert(!pm_is_suppressed(&pm, 1002));
    pm_destroy(&pm);
}

static void test_unsuppress(void) {
    ProcessManager pm;
    pm_init(&pm);
    pm_add(&pm, 2001);
    pm_suppress(&pm, 2001);
    assert(pm_is_suppressed(&pm, 2001));
    pm_unsuppress(&pm, 2001);
    assert(!pm_is_suppressed(&pm, 2001));
    pm_destroy(&pm);
}

static void test_list_suppressed(void) {
    ProcessManager pm;
    pm_init(&pm);
    for (uint32_t i = 3000; i < 3005; ++i) pm_add(&pm, i);
    pm_suppress(&pm, 3001);
    pm_suppress(&pm, 3003);
    uint32_t buffer[10];
    size_t count = pm_list_suppressed(&pm, buffer, 10);
    assert(count == 2);
    assert((buffer[0] == 3001 && buffer[1] == 3003) ||
           (buffer[0] == 3003 && buffer[1] == 3001));
    pm_destroy(&pm);
}

static void benchmark(void) {
    ProcessManager pm;
    pm_init(&pm);
    const size_t N = 100000;
    clock_t start = clock();
    for (uint32_t i = 1; i <= N; ++i) pm_add(&pm, i);
    for (uint32_t i = 1; i <= N; i += 2) pm_suppress(&pm, i);
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Benchmark: Added %zu processes and suppressed %zu in %.3f seconds.\n",
           N, N / 2, elapsed);
    pm_destroy(&pm);
}

int main(void) {
    test_add_and_suppress();
    test_unsuppress();
    test_list_suppressed();
    printf("All unit tests passed.\n");
    benchmark();
    return 0;
}
