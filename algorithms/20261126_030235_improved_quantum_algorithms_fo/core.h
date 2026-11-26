#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * Computes the optimal Hamiltonian cycle (Traveling Salesman Problem) using
 * the Held‑Karp dynamic programming algorithm.
 *
 * Parameters
 *   dist      : pointer to an n×n matrix stored row‑major (dist[i*n + j])
 *   n         : number of cities (0 ≤ n ≤ TSP_MAX_N)
 *   opt_cost  : output parameter receiving the minimal tour length
 *   tour_out  : output array of length n+1 receiving the optimal tour
 *               (tour_out[0] == tour_out[n] == 0). May be NULL if only the
 *               cost is required.
 *
 * Returns 0 on success, -1 on error (e.g., n > TSP_MAX_N or allocation failure).
 */
int tsp_held_karp(const double *dist, size_t n, double *opt_cost, uint32_t *tour_out);

#ifdef __cplusplus
}
#endif
