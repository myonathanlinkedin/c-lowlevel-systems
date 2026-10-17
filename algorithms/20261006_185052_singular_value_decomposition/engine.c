#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "core.h"

static void matrix_set_internal(Matrix* m, size_t i, size_t j, double val) {
    m->data[i * m->cols + j] = val;
}

static double matrix_get_internal(const Matrix* m, size_t i, size_t j) {
    return m->data[i * m->cols + j];
}

Matrix* matrix_create(size_t rows, size_t cols) {
    Matrix* m = (Matrix*)malloc(sizeof(Matrix));
    if (!m) return NULL;
    m->rows = rows;
    m->cols = cols;
    m->data = (double*)calloc(rows * cols, sizeof(double));
    if (!m->data) {
        free(m);
        return NULL;
    }
    return m;
}

void matrix_destroy(Matrix* m) {
    if (!m) return;
    free(m->data);
    free(m);
}

void matrix_set(Matrix* m, size_t i, size_t j, double val) {
    if (i >= m->rows || j >= m->cols) return;
    matrix_set_internal(m, i, j, val);
}

double matrix_get(const Matrix* m, size_t i, size_t j) {
    if (i >= m->rows || j >= m->cols) return 0.0;
    return matrix_get_internal(m, i, j);
}

Matrix* matrix_copy(const Matrix* m) {
    Matrix* copy = matrix_create(m->rows, m->cols);
    if (!copy) return NULL;
    memcpy(copy->data, m->data, m->rows * m->cols * sizeof(double));
    return copy;
}

Matrix* matrix_transpose(const Matrix* m) {
    Matrix* t = matrix_create(m->cols, m->rows);
    if (!t) return NULL;
    for (size_t i = 0; i < m->rows; ++i)
        for (size_t j = 0; j < m->cols; ++j)
            matrix_set_internal(t, j, i, matrix_get_internal(m, i, j));
    return t;
}

Matrix* matrix_multiply(const Matrix* a, const Matrix* b) {
    if (a->cols != b->rows) return NULL;
    Matrix* r = matrix_create(a->rows, b->cols);
    if (!r) return NULL;
    for (size_t i = 0; i < a->rows; ++i) {
        for (size_t j = 0; j < b->cols; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < a->cols; ++k)
                sum += matrix_get_internal(a, i, k) * matrix_get_internal(b, k, j);
            matrix_set_internal(r, i, j, sum);
        }
    }
    return r;
}

Matrix* matrix_scale(const Matrix* m, double scalar) {
    Matrix* r = matrix_create(m->rows, m->cols);
    if (!r) return NULL;
    for (size_t i = 0; i < m->rows * m->cols; ++i)
        r->data[i] = m->data[i] * scalar;
    return r;
}

Matrix* matrix_subtract(const Matrix* a, const Matrix* b) {
    if (a->rows != b->rows || a->cols != b->cols) return NULL;
    Matrix* r = matrix_create(a->rows, a->cols);
    if (!r) return NULL;
    for (size_t i = 0; i < a->rows * a->cols; ++i)
        r->data[i] = a->data[i] - b->data[i];
    return r;
}

Matrix* matrix_identity(size_t n) {
    Matrix* I = matrix_create(n, n);
    if (!I) return NULL;
    for (size_t i = 0; i < n; ++i)
        matrix_set_internal(I, i, i, 1.0);
    return I;
}

double matrix_frobenius_norm(const Matrix* m) {
    double sum = 0.0;
    for (size_t i = 0; i < m->rows * m->cols; ++i)
        sum += m->data[i] * m->data[i];
    return sqrt(sum);
}

/* Power iteration for symmetric matrix A (n x n) */
static void power_iteration(const Matrix* A, size_t max_iter, double* eigenvalue, double* eigenvector) {
    size_t n = A->rows;
    double* b = (double*)calloc(n, sizeof(double));
    double* b_next = (double*)calloc(n, sizeof(double));
    for (size_t i = 0; i < n; ++i) b[i] = 1.0; // initial vector
    for (size_t iter = 0; iter < max_iter; ++iter) {
        // b_next = A * b
        for (size_t i = 0; i < n; ++i) {
            double sum = 0.0;
            for (size_t j = 0; j < n; ++j)
                sum += matrix_get_internal(A, i, j) * b[j];
            b_next[i] = sum;
        }
        // normalize b_next
        double norm = 0.0;
        for (size_t i = 0; i < n; ++i)
            norm += b_next[i] * b_next[i];
        norm = sqrt(norm);
        if (norm == 0.0) break;
        for (size_t i = 0; i < n; ++i)
            b_next[i] /= norm;
        // check convergence
        double diff = 0.0;
        for (size_t i = 0; i < n; ++i)
            diff += fabs(b_next[i] - b[i]);
        if (diff < 1e-10) break;
        // swap
        double* tmp = b;
        b = b_next;
        b_next = tmp;
    }
    // compute eigenvalue = (b^T * A * b)
    double num = 0.0, den = 0.0;
    for (size_t i = 0; i < n; ++i) {
        double sum = 0.0;
        for (size_t j = 0; j < n; ++j)
            sum += matrix_get_internal(A, i, j) * b[j];
        num += b[i] * sum;
        den += b[i] * b[i];
    }
    *eigenvalue = num / den;
    for (size_t i = 0; i < n; ++i)
        eigenvector[i] = b[i];
    free(b);
    free(b_next);
}

/* Deflate matrix B by subtracting lambda * v * v^T */
static void deflate(Matrix* B, double lambda, const double* v) {
    size_t n = B->rows;
    for (size_t i = 0; i < n; ++i)
        for (size_t j = 0; j < n; ++j)
            B->data[i * n + j] -= lambda * v[i] * v[j];
}

int compute_svd(const Matrix* A, size_t k, Matrix** U_out, Matrix** S_out, Matrix** V_out) {
    if (!A || k == 0) return -1;
    size_t m = A->rows, n = A->cols;
    Matrix* At = matrix_transpose(A);
    Matrix* B = matrix_multiply(At, A); // B = A^T A
    if (!At || !B) { matrix_destroy(At); matrix_destroy(B
);

}
}
