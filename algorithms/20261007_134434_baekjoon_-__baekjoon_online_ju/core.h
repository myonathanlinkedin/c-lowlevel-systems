#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include "types.h"

void fenwick_init(FenwickTree *ft, size_t n);
void fenwick_destroy(FenwickTree *ft);
void fenwick_update(FenwickTree *ft, size_t idx, int64_t delta);
int64_t fenwick_query(FenwickTree *ft, size_t idx);
int64_t fenwick_range_query(FenwickTree *ft, size_t l, size_t r);
