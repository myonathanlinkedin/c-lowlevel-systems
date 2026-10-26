#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "core.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

/* Matrix utilities */
Matrix matrix_create(size_t rows, size_t cols) {
    Matrix m;
    m.rows = rows;
    m.cols = cols;
    m.data = (double *)calloc(rows * cols, sizeof(double));
    assert(m.data != NULL);
    return m;
}

void matrix_destroy(Matrix *m) {
    if (m->data) {
        free(m->data);
        m->data = NULL;
    }
    m->rows = 0;
    m->cols = 0;
}

Matrix matrix_copy(const Matrix *src) {
    Matrix dst = matrix_create(src->rows, src->cols);
    memcpy(dst.data, src->data, src->rows * src->cols * sizeof(double));
    return dst;
}

Matrix matrix_transpose(const Matrix *src) {
    Matrix dst = matrix_create(src->cols, src->rows);
    for (size_t i = 0; i < src->rows; ++i) {
        for (size_t j = 0; j < src->cols; ++j) {
            dst.data[j * dst.cols + i] = src->data[i * src->cols + j];
        }
    }
    return dst;
}

Matrix matrix_multiply(const Matrix *A, const Matrix *B) {
    assert(A->cols == B->rows);
    Matrix C = matrix_create(A->rows, B->cols);
    for (size_t i = 0; i < A->rows; ++i) {
        for (size_t k = 0; k < A->cols; ++k) {
            double aik = A->data[i * A->cols + k];
            for (size_t j = 0; j < B->cols; ++j) {
                C.data[i * C.cols + j] += aik * B->data[k * B->cols + j];
            }
        }
    }
    return C;
}

Matrix matrix_scale(const Matrix *A, double scalar) {
    Matrix B = matrix_copy(A);
    for (size_t i = 0; i < A->rows * A->cols; ++i) {
        B.data[i] *= scalar;
    }
    return B;
}

Matrix matrix_subtract(const Matrix *A, const Matrix *B) {
    assert(A->rows == B->rows && A->cols == B->cols);
    Matrix C = matrix_create(A->rows, A->cols);
    for (size_t i = 0; i < A->rows * A->cols; ++i) {
        C.data[i] = A->data[i] - B->data[i];
    }
    return C;
}

double matrix_norm_frobenius(const Matrix *A) {
    double sum = 0.0;
    for (size_t i = 0; i < A->rows * A->cols; ++i) {
        sum += A->data[i] * A->data[i];
    }
    return sqrt(sum);
}

/* Vector utilities */
double *vector_alloc(size_t n) {
    double *v = (double *)calloc(n, sizeof(double));
    assert(v != NULL);
    return v;
}

void vector_free(double *v) {
    free(v);
}

double vector_norm(const double *v, size_t n) {
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i) {
        sum += v[i] * v[i];
    }
    return sqrt(sum);
}

double vector_dot(const double *a, const double *b, size_t n) {
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

void vector_scale(double *v, double scalar, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        v[i] *= scalar;
    }
}

void vector_normalize(double *v, size_t n) {
    double norm = vector_norm(v, n);
    if (norm > 0.0) {
        vector_scale(v, 1.0 / norm, n);
    }
}

void vector_copy(double *dest, const double *src, size_t n) {
    memcpy(dest, src, n * sizeof(double));
}

/* Helper: outer product of vector with itself */
static Matrix outer_product(const double *v, size_t n) {
    Matrix M = matrix_create(n, n);
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            M.data[i * n + j] = v[i] * v[j];
        }
    }
    return M;
}

/* Helper: power iteration for dominant eigenvector */
static bool power_iteration(const Matrix *B, double *eigval, double *eigvec, size_t n, size_t max_iter, double tol) {
    double *x = vector_alloc(n);
    for (size_t i = 0; i < n; ++i) x[i] = 1.0; /* deterministic start */
    vector_normalize(x, n);
    double lambda_old = 0.0;
    for (size_t iter = 0; iter < max_iter; ++iter) {
        /* y = B * x */
        double *y = vector_alloc(n);
        for (size_t i = 0; i < n; ++i) {
            double sum = 0.0;
            for (size_t j = 0; j < n; ++j) {
                sum += B->data[i * n + j] * x[j];
            }
            y[i] = sum;
        }
        double lambda = vector_dot(y, x, n);
        vector_normalize(y, n);
        double diff = 0.0;
        for (size_t i = 0; i < n; ++i) {
            diff += fabs(y[i] - x[i]);
        }
        vector_free(x);
        x = y;
        if (fabs(lambda - lambda_old) < tol) {
            break;
        }
        lambda_old = lambda;
    }
    *eigval = lambda_old;
    vector_copy(eigvec, x, n);
    vector_free(x);
    return true;
}

/* SVD low-rank approximation */
bool svd_lowrank(const Matrix *A, size_t k, SVDResult *result) {
    if (k == 0 || A->rows == 0 || A->cols == 0) return false;
    size_t m = A->rows;
    size_t n = A->cols;
    size_t r = (k < (m < n ? m : n)) ? k : (m < n ? m : n);
    /* Compute B = A^T * A */
    Matrix At = matrix_transpose(A);
    Matrix B = matrix_multiply(&At, A);
    matrix_destroy(&At);
    /* Allocate storage for eigenvectors */
    double **eigvecs = (double **)malloc(r * sizeof(double *));
    double *eigvals = (double *)malloc(r * sizeof(double));
    assert(eigvecs != NULL && eigvals != NULL);
    /* Power iteration with deflation */
    for (size_t i = 0; i < r; ++i) {
        double *vec = vector_alloc(n);
        double lambda = 0.0;
        power_iteration(&B, &lambda, vec, n, 1000, 1e-10);
        eigvals[i] = lambda;
        eigvecs[i] = vec;
        /* Deflate B: B = B - lambda * v * v^T */
        Matrix outer = outer_product(vec, n);
        Matrix lambda_outer = matrix_scale(&outer, lambda);
        Matrix B_new = matrix_subtract(&B, &lambda_outer);
        matrix_destroy(&B);
        matrix_destroy(&outer);
        matrix_destroy(&lambda_outer);
        B = B_new;
    }
    /* Build V matrix from eigenvectors */
    Matrix V = matrix_create(n, r);
    for (size_t i = 0; i < r; ++i) {
        for (size_t j = 0; j < n; ++j) {
            V.data[j * r + i] = eigvecs[i][j];
        }
    }
    /* Singular values */
    double *S = (double *)malloc(r * sizeof(double));
    for (size_t i = 0; i < r; ++i) {
        S[i] = sqrt(fabs(eigvals[i]));
    }
    /* Compute U = A * V * diag(1/sigma) */
    Matrix AV = matrix_multiply(A, &V);
    Matrix U = matrix_create(m, r);
    for (size_t i = 0; i < r; ++i) {
        double inv_sigma = (S[i] > 1e-12) ? (1.0 / S[i]) : 0.0;
        for (size_t j = 0; j < m; ++j) {
            U.data[j * r + i] = AV.data[j * r + i] * inv_sigma;
        }
    }
    /* Transpose V to get Vt */
    Matrix Vt = matrix_transpose(&V);
    /* Clean up */
    for (size_t i = 0; i < r; ++i) {
        vector_free(eigvecs[i]);
    }
    free(eigvecs);
    free(eigvals);
    matrix_destroy(&B);
    matrix_destroy(&V);
    matrix_destroy(&AV);
    /* Fill result */
    result->U = U;
    result->S = S;
    result->Vt = Vt;
    result->rank = r;
    return true;
}

void svd_lowrank_destroy(SVDResult *result) {
    matrix_destroy(&result->U);
    matrix_destroy(&result->Vt);
    vector_free(result->S);
    result->rank = 0;
}
