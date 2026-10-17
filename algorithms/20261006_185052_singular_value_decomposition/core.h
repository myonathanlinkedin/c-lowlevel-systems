#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include "types.h"

Matrix* matrix_create(size_t rows, size_t cols);
void matrix_destroy(Matrix* m);
void matrix_set(Matrix* m, size_t i, size_t j, double val);
double matrix_get(const Matrix* m, size_t i, size_t j);
Matrix* matrix_copy(const Matrix* m);
Matrix* matrix_transpose(const Matrix* m);
Matrix* matrix_multiply(const Matrix* a, const Matrix* b);
Matrix* matrix_scale(const Matrix* m, double scalar);
Matrix* matrix_subtract(const Matrix* a, const Matrix* b);
Matrix* matrix_identity(size_t n);
double matrix_frobenius_norm(const Matrix* m);

int compute_svd(const Matrix* A, size_t k, Matrix** U, Matrix** S, Matrix** V);
Matrix* low_rank_approximation(const Matrix* A, size_t k);
