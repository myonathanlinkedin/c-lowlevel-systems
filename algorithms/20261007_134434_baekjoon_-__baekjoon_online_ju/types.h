#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    size_t size;
    int64_t *tree;
} FenwickTree;

void fenwick_init(FenwickTree *ft, size_t n);
void fenwick_destroy(FenwickTree *ft);
void fenwick_update(FenwickTree *ft, size_t idx, int64_t delta);
int64_t fenwick_query(FenwickTree *ft, size_t idx);
int64_t fenwick_range_query(FenwickTree *ft, size_t l, size_t r);
