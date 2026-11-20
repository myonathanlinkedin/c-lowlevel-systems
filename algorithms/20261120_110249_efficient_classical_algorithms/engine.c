#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "types.h"
#include "core.h"

Matrix* matrix_alloc(size_t rows, size_t cols) {
    Matrix *m = malloc(sizeof(Matrix));
    assert(m != NULL);
    m->rows = rows;
    m->cols = cols;
    m->data = calloc(rows * cols, sizeof(double));
    assert(m->data != NULL);
    return m;
}

void matrix_free(Matrix *m) {
    if (m) {
        free(m->data);
        free(m);
    }
}

void matrix_set(Matrix *m, size_t row, size_t col, double value) {
    assert(m != NULL);
    assert(row < m->rows);
    assert(col < m->cols);
    m->data[row * m->cols + col] = value;
}

double matrix_get(const Matrix *m, size_t row, size_t col) {
    assert(m != NULL);
    assert(row < m->rows);
    assert(col < m->cols);
    return m->data[row * m->cols + col];
}

Matrix* matrix_copy(const Matrix *src) {
    assert(src != NULL);
    Matrix *dst = matrix_alloc(src->rows, src->cols);
    memcpy(dst->data, src->data, src->rows * src->cols * sizeof(double));
    return dst;
}

void matrix_print(const Matrix *m) {
    assert(m != NULL);
    for (size_t i = 0; i < m->rows; ++i) {
        for (size_t j = 0; j < m->cols; ++j) {
            printf("%10.4f ", matrix_get(m, i, j));
        }
        printf("\n");
    }
}

static void jacobi_rotate(double *A, size_t n, size_t p, size_t q, double c, double s) {
    for (size_t i = 0; i < n; ++i) {
        if (i != p && i != q) {
            double aip = A[i * n + p];
            double aiq = A[i * n + q];
            A[i * n + p] = c * aip - s * aiq;
            A[i * n + q] = s * aip + c * aiq;
            A[p * n + i] = A[i * n + p];
            A[q * n + i] = A[i * n + q];
        }
    }
    double app = A[p * n + p];
    double aqq = A[q * n + q];
    double apq = A[p * n + q];
    double app_new = c * c * app - 2.0 * s * c * apq + s * s * aqq;
    double aqq_new = s * s * app + 2.0 * s * c * apq + c * c * aqq;
    A[p * n + p] = app_new;
    A[q * n + q] = aqq_new;
    A[p * n + q] = 0.0;
    A[q * n + p] = 0.0;
}

void matrix_jacobi_eigenvalues(const Matrix *m, double *eigvals, size_t max_iter, double eps) {
    assert(m != NULL);
    assert(m->rows == m->cols);
    size_t n = m->rows;
    double *A = malloc(n * n * sizeof(double));
    assert(A != NULL);
    memcpy(A, m->data, n * n * sizeof(double));
    for (size_t i = 0; i < n; ++i) {
        eigvals[i] = A[i * n + i];
    }
    for (size_t iter = 0; iter < max_iter; ++iter) {
        double max_val = 0.0;
        size_t p = 0, q = 1;
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = i + 1; j < n; ++j) {
                double val = fabs(A[i * n + j]);
                if (val > max_val) {
                    max_val = val;
                    p = i;
                    q = j;
                }
            }
        }
        if (max_val < eps) {
            break;
        }
        double app = A[p * n + p];
        double aqq = A[q * n + q];
        double apq = A[p * n + q];
        double phi = 0.5 * atan2(2.0 * apq, aqq - app);
        double c = cos(phi);
        double s = sin(phi);
        jacobi_rotate(A, n, p, q, c, s);
    }
    for (size_t i = 0; i < n; ++i) {
        eigvals[i] = A[i * n + i];
    }
    free(A);
}

double matrix_ground_energy(const Matrix *m) {
    assert(m != NULL);
    assert(m->rows == m->cols);
    size_t n = m->rows;
    double *eigvals = malloc(n * sizeof(double));
    assert(eigvals != NULL);
    matrix_jacobi_eigenvalues(m, eigvals, 100 * n * n, 1e-10);
    double min_val = eigvals[0];
    for (size_t i = 1; i < n; ++i) {
        if (eigvals[i] < min_val) {
            min_val = eigvals[i];
        }
    }
    free(eigvals);
    return min_val;
}
