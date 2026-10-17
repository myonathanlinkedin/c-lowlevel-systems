#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "types.h"

/* Create a matrix with given dimensions, zero-initialized */
bool matrix_create(Matrix *m, size_t rows, size_t cols) {
    if (!m) return false;
    m->rows = rows;
    m->cols = cols;
    m->data = (double *)calloc(rows * cols, sizeof(double));
    return m->data != NULL;
}

void matrix_destroy(Matrix *m) {
    if (m && m->data) {
        free(m->data);
        m->data = NULL;
    }
}

/* Create a vector with given size, zero-initialized */
bool vector_create(Vector *v, size_t size) {
    if (!v) return false;
    v->size = size;
    v->data = (double *)calloc(size, sizeof(double));
    return v->data != NULL;
}

void vector_destroy(Vector *v) {
    if (v && v->data) {
        free(v->data);
        v->data = NULL;
    }
}

/* Multiply matrix A by vector x, store result in y */
bool matrix_vector_mul(const Matrix *A, const Vector *x, Vector *y) {
    if (!A || !x || !y) return false;
    if (A->cols != x->size || A->rows != y->size) return false;
    for (size_t i = 0; i < A->rows; ++i) {
        double sum = 0.0;
        for (size_t j = 0; j < A->cols; ++j) {
            sum += A->data[i * A->cols + j] * x->data[j];
        }
        y->data[i] = sum;
    }
    return true;
}

/* Compute Euclidean norm of vector */
double vector_norm(const Vector *v) {
    if (!v) return 0.0;
    double sum = 0.0;
    for (size_t i = 0; i < v->size; ++i) {
        sum += v->data[i] * v->data[i];
    }
    return sqrt(sum);
}

/* Normalize vector to unit length */
void vector_normalize(Vector *v) {
    if (!v) return;
    double norm = vector_norm(v);
    if (norm > 0.0) {
        for (size_t i = 0; i < v->size; ++i) {
            v->data[i] /= norm;
        }
    }
}

/* Dot product of two vectors */
double vector_dot(const Vector *a, const Vector *b) {
    if (!a || !b || a->size != b->size) return 0.0;
    double sum = 0.0;
    for (size_t i = 0; i < a->size; ++i) {
        sum += a->data[i] * b->data[i];
    }
    return sum;
}

/* Scale vector by scalar */
void vector_scale(Vector *v, double scalar) {
    if (!v) return;
    for (size_t i = 0; i < v->size; ++i) {
        v->data[i] *= scalar;
    }
}

/* Add vector b to vector a (a += b) */
void vector_add(Vector *a, const Vector *b) {
    if (!a || !b || a->size != b->size) return;
    for (size_t i = 0; i < a->size; ++i) {
        a->data[i] += b->data[i];
    }
}

/* Power iteration to find leading eigenvector of symmetric matrix A */
bool power_iteration(const Matrix *A, Vector *eigenvector, double tol, size_t max_iter) {
    if (!A || !eigenvector) return false;
    if (A->rows != A->cols || A->rows != eigenvector->size) return false;

    /* Temporary vector for iteration */
    Vector y;
    if (!vector_create(&y, A->rows)) return false;

    /* Initialize x with random values */
    for (size_t i = 0; i < eigenvector->size; ++i) {
        eigenvector->data[i] = ((double)rand() / RAND_MAX) - 0.5;
    }
    vector_normalize(eigenvector);

    double lambda_old = 0.0;
    for (size_t iter = 0; iter < max_iter; ++iter) {
        /* y = A * x */
        if (!matrix_vector_mul(A, eigenvector, &y)) {
            vector_destroy(&y);
            return false;
        }
        /* Normalize y to get next x */
        vector_normalize(&y);
        /* Rayleigh quotient for eigenvalue estimate */
        double lambda_new = vector_dot(eigenvector, &y);
        /* Check convergence */
        if (fabs(lambda_new - lambda_old) < tol) {
            /* Copy y into eigenvector */
            for (size_t i = 0; i < eigenvector->size; ++i) {
                eigenvector->data[i] = y.data[i];
            }
            vector_destroy(&y);
            return true;
        }
        /* Prepare for next iteration */
        for (size_t i = 0; i < eigenvector->size; ++i) {
            eigenvector->data[i] = y.data[i];
        }
        lambda_old = lambda_new;
    }
    vector_destroy(&y);
    return false; /* did not converge within max_iter */
}
