#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include <stdio.h>
#include <assert.h>
#include "core.h"

/* Test row‑major 2‑D layout offset calculation. */
static void test_row_major_2d(void) {
    size_t dims[2] = {4, 5};               // 4 rows, 5 columns
    Layout *L = layout_create(2, dims, true);
    assert(L != NULL);
    assert(layout_is_valid(L));

    size_t idx[2] = {2, 3};                // row 2, col 3
    size_t off = layout_offset(L, idx);
    assert(off == 2 * 5 + 3);             // expected 13

    layout_destroy(L);
}

/* Test column‑major 2‑D layout offset calculation. */
static void test_column_major_2d(void) {
    size_t dims[2] = {4, 5};
    Layout *L = layout_create(2, dims, false);
    assert(L != NULL);
    size_t idx[2] = {2, 3};
    size_t off = layout_offset(L, idx);
    assert(off == 3 * 4 + 2);             // expected 14
    layout_destroy(L);
}

/* Verify that transposition preserves physical element ordering. */
static void test_transpose(void) {
    size_t dims[2] = {3, 2};
    Layout *L = layout_create(2, dims, true);
    size_t perm[2] = {1, 0};
    Layout *T = layout_transpose(L, perm);
    assert(T != NULL);

    size_t idx_orig[2] = {1, 0};           // element (1,0) in original
    size_t off_orig = layout_offset(L, idx_orig);

    size_t idx_t[2] = {0, 1};              // same element after transpose
    size_t off_t = layout_offset(T, idx_t);
    assert(off_orig == off_t);

    layout_destroy(L);
    layout_destroy(T);
}

/* Reshape a 2‑D layout to a 3‑D layout and verify offset equivalence. */
static void test_reshape(void) {
    size_t dims[2] = {2, 3};
    Layout *L = layout_create(2, dims, true);
    size_t new_dims[3] = {3, 2, 1};
    Layout *R = layout_reshape(L, 3, new_dims);
    assert(R != NULL);

    size_t idx_orig[2] = {1, 2};
    size_t off_orig = layout_offset(L, idx_orig);   // 1*3+2 = 5

    size_t idx_new[3] = {2, 1, 0};
    size_t off_new = layout_offset(R, idx_new);
    assert(off_orig == off_new);

    layout_destroy(L);
    layout_destroy(R);
}

/* Ensure creation fails for zero‑sized dimensions. */
static void test_invalid(void) {
    size_t dims[2] = {0, 5};
    Layout *L = layout_create(2, dims, true);
    assert(L == NULL || !layout_is_valid(L));
    if (L) layout_destroy(L);
}

/* Entry point – run all unit tests. */
int main(void) {
    test_row_major_2d();
    test_column_major_2d();
    test_transpose();
    test_reshape();
    test_invalid();
    printf("All layout algebra tests passed.\n");
    return 0;
}
