#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "core.h"

void fenwick_init(FenwickTree *ft, size_t n) {
    ft->size = n;
    ft->tree = calloc(n + 1, sizeof(int64_t));
}

void fenwick_destroy(FenwickTree *ft) {
    free(ft->tree);
    ft->tree = NULL;
    ft->size = 0;
}

void fenwick_update(FenwickTree *ft, size_t idx, int64_t delta) {
    for (size_t i = idx; i <= ft->size; i += (i & -i)) {
        ft->tree[i] += delta;
    }
}

int64_t fenwick_query(FenwickTree *ft, size_t idx) {
    int64_t sum = 0;
    for (size_t i = idx; i > 0; i -= (i & -i)) {
        sum += ft->tree[i];
    }
    return sum;
}

int64_t fenwick_range_query(FenwickTree *ft, size_t l, size_t r) {
    if (l > r) return 0;
    return fenwick_query(ft, r) - fenwick_query(ft, l - 1);
}
