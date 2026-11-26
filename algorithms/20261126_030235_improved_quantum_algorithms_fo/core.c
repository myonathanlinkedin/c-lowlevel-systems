#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdio.h>

static inline size_t dp_index(size_t mask, size_t i, size_t n) {
    return mask * n + i;
}

/* Held‑Karp DP implementation */
int tsp_held_karp(const double *dist, size_t n, double *opt_cost, uint32_t *tour_out) {
    if (n > TSP_MAX_N) return -1;

    if (n == 0) {
        if (opt_cost) *opt_cost = 0.0;
        if (tour_out) tour_out[0] = 0;
        return 0;
    }
    if (n == 1) {
        if (opt_cost) *opt_cost = 0.0;
        if (tour_out) {
            tour_out[0] = 0;
            tour_out[1] = 0;
        }
        return 0;
    }

    size_t full_mask = ((size_t)1 << n) - 1;
    size_t dp_elems = (full_mask + 1) * n;

    double *dp = (double *)malloc(dp_elems * sizeof(double));
    int    *parent = (int *)malloc(dp_elems * sizeof(int));
    if (!dp || !parent) {
        free(dp);
        free(parent);
        return -1;
    }

    for (size_t i = 0; i < dp_elems; ++i) {
        dp[i] = DBL_MAX;
        parent[i] = -1;
    }

    dp[dp_index(1 << 0, 0, n)] = 0.0;   // start at city 0

    for (size_t mask = 1; mask <= full_mask; ++mask) {
        if (!(mask & 1)) continue;      // must contain start city 0
        for (size_t i = 0; i < n; ++i) {
            if (!(mask & ((size_t)1 << i))) continue;
            double cur = dp[dp_index(mask, i, n)];
            if (cur == DBL_MAX) continue;
            for (size_t j = 0; j < n; ++j) {
                if (mask & ((size_t)1 << j)) continue;
                size_t next_mask = mask | ((size_t)1 << j);
                double ndist = cur + dist[i * n + j];
                size_t pos = dp_index(next_mask, j, n);
                if (ndist < dp[pos]) {
                    dp[pos] = ndist;
                    parent[pos] = (int)i;
                }
            }
        }
    }

    double best = DBL_MAX;
    int best_last = -1;
    for (size_t i = 1; i < n; ++i) {
        double cand = dp[dp_index(full_mask, i, n)] + dist[i * n + 0];
        if (cand < best) {
            best = cand;
            best_last = (int)i;
        }
    }

    if (opt_cost) *opt_cost = best;

    if (tour_out) {
        uint32_t rev[TSP_MAX_N + 1];
        size_t mask = full_mask;
        int cur = best_last;
        rev[n] = 0;                     // final return to start

        for (int pos = (int)n - 1; pos >= 1; --pos) {
            rev[pos] = (uint32_t)cur;
            int prev = parent[dp_index(mask, cur, n)];
            mask ^= ((size_t)1 << cur);
            cur = prev;
        }
        rev[0] = 0;                     // start city

        for (size_t i = 0; i <= n; ++i) {
            tour_out[i] = rev[i];
        }
    }

    free(dp);
    free(parent);
    return 0;
}
