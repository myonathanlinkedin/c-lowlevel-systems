#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#pragma once
#include "types.h"

/* Matrix utilities */
Matrix matrix_create(size_t rows, size_t cols);
void matrix_destroy(Matrix *m);
Matrix matrix_copy(const Matrix *src);
Matrix matrix_transpose(const Matrix *src);
Matrix matrix_multiply(const Matrix *A, const Matrix *B);
Matrix matrix_scale(const Matrix *A, double scalar);
Matrix matrix_subtract(const Matrix *A, const Matrix *B);
double matrix_norm_frobenius(const Matrix *A);

/* Vector utilities */
double *vector_alloc(size_t n);
void vector_free(double *v);
double vector_norm(const double *v, size_t n);
double vector_dot(const double *a, const double *b, size_t n);
void vector_scale(double *v, double scalar, size_t n);
void vector_normalize(double *v, size_t n);
void vector_copy(double *dest, const double *src, size_t n);

/* SVD */
bool svd_lowrank(const Matrix *A, size_t k, SVDResult *result);
void svd_lowrank_destroy(SVDResult *result);
