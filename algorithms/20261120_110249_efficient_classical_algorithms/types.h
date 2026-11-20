#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <math.h>

typedef struct {
    size_t rows;
    size_t cols;
    double *data;
} Matrix;

Matrix* matrix_alloc(size_t rows, size_t cols);
void matrix_free(Matrix *m);
void matrix_set(Matrix *m, size_t row, size_t col, double value);
double matrix_get(const Matrix *m, size_t row, size_t col);
Matrix* matrix_copy(const Matrix *src);
void matrix_print(const Matrix *m);
