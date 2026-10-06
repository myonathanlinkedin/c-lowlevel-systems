#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdlib.h>
#include <math.h>

double comb(int n, int k) {
    if (k > n) return 0.0;
    if (k > n - k) k = n - k;
    double result = 1.0;
    for (int i = 1; i <= k; ++i) {
        result *= (double)(n - k + i) / (double)i;
    }
    return result;
}

void random_subset(int n, int r, int *subset) {
    int *indices = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; ++i) indices[i] = i;
    for (int i = 0; i < r; ++i) {
        int j = i + rand() % (n - i);
        int tmp = indices[i];
        indices[i] = indices[j];
        indices[j] = tmp;
        subset[i] = indices[i];
    }
    free(indices);
}

bool uniform_is_independent(const int *set, size_t k, const Matroid *m) {
    (void)set;
    return k <= m->rank;
}

void approximate_common_bases(const Matroid *m1, const Matroid *m2,
                              const FPRASParams *params, ApproxResult *res) {
    size_t n = m1->ground_set_size;
    size_t r = m1->rank;
    if (m2->ground_set_size != n || m2->rank != r) {
        res->estimate = 0.0;
        res->error = 0.0;
        return;
    }
    double total_subsets = comb((int)n, (int)r);
    size_t successes = 0;
    int *subset = (int *)malloc(r * sizeof(int));
    for (size_t i = 0; i < params->samples; ++i) {
        random_subset((int)n, (int)r, subset);
        if (m1->is_independent(subset, r, m1) &&
            m2->is_independent(subset, r, m2)) {
            ++successes;
        }
    }
    free(subset);
    double p = (double)successes / (double)params->samples;
    res->estimate = p * total_subsets;
    double var = p * (1.0 - p) / (double)params->samples;
    res->error = sqrt(var) * total_subsets;
}
