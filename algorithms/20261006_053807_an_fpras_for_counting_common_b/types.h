#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef struct Matroid {
    size_t ground_set_size;
    size_t rank;
    bool (*is_independent)(const int *set, size_t k, const struct Matroid *m);
} Matroid;

typedef struct FPRASParams {
    size_t samples;
    double tolerance;
} FPRASParams;

typedef struct ApproxResult {
    double estimate;
    double error;
} ApproxResult;

double comb(int n, int k);
void random_subset(int n, int r, int *subset);
bool uniform_is_independent(const int *set, size_t k, const Matroid *m);
void approximate_common_bases(const Matroid *m1, const Matroid *m2,
                              const FPRASParams *params, ApproxResult *res);
