#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include "types.h"

void matrix_jacobi_eigenvalues(const Matrix *m, double *eigvals, size_t max_iter, double eps);
double matrix_ground_energy(const Matrix *m);
