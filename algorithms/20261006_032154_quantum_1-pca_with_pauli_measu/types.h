#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#ifndef TYPES_H
#define TYPES_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

/* Matrix and vector structures */
typedef struct {
    size_t rows;
    size_t cols;
    double *data; /* row-major order */
} Matrix;

typedef struct {
    size_t size;
    double *data;
} Vector;

/* Function prototypes */
bool matrix_create(Matrix *m, size_t rows, size_t cols);
void matrix_destroy(Matrix *m);
bool vector_create(Vector *v, size_t size);
void vector_destroy(Vector *v);
bool matrix_vector_mul(const Matrix *A, const Vector *x, Vector *y);
double vector_norm(const Vector *v);
void vector_normalize(Vector *v);
double vector_dot(const Vector *a, const Vector *b);
void vector_scale(Vector *v, double scalar);
void vector_add(Vector *a, const Vector *b);
bool power_iteration(const Matrix *A, Vector *eigenvector, double tol, size_t max_iter);

#endif /* TYPES_H */
