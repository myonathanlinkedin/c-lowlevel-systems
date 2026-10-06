#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include "types.h"
#include <stdio.h>
#include <math.h>
#include <time.h>

/* Helper to set matrix data */
void set_matrix(Matrix *m, const double *values) {
    assert(m && values);
    for (size_t i = 0; i < m->rows * m->cols; ++i) {
        m->data[i] = values[i];
    }
}

/* Helper to compare vectors within tolerance */
bool vectors_close(const Vector *a, const Vector *b, double tol) {
    if (!a || !b || a->size != b->size) return false;
    for (size_t i = 0; i < a->size; ++i) {
        if (fabs(a->data[i] - b->data[i]) > tol) return false;
    }
    return true;
}

/* Compute eigenvalue via Rayleigh quotient */
double eigenvalue(const Matrix *A, const Vector *v) {
    Vector y;
    vector_create(&y, A->rows);
    matrix_vector_mul(A, v, &y);
    double val = vector_dot(v, &y);
    vector_destroy(&y);
    return val;
}

int main(void) {
    srand((unsigned)time(NULL));

    /* Test 1: Identity matrix */
    Matrix I;
    matrix_create(&I, 3, 3);
    double I_vals[9] = {1,0,0, 0,1,0, 0,0,1};
    set_matrix(&I, I_vals);
    Vector eig1;
    vector_create(&eig1, 3);
    power_iteration(&I, &eig1, 1e-6, 1000);
    double val1 = eigenvalue(&I, &eig1);
    assert(fabs(val1 - 1.0) < 1e-4);
    assert(fabs(vector_norm(&eig1) - 1.0) < 1e-6);

    /* Test 2: Diagonal matrix diag(3,2,1) */
    Matrix D;
    matrix_create(&D, 3, 3);
    double D_vals[9] = {3,0,0, 0,2,0, 0,0,1};
    set_matrix(&D, D_vals);
    Vector eig2;
    vector_create(&eig2, 3);
    power_iteration(&D, &eig2, 1e-6, 1000);
    double val2 = eigenvalue(&D, &eig2);
    assert(fabs(val2 - 3.0) < 1e-4);
    Vector expected2;
    vector_create(&expected2, 3);
    expected2.data[0] = 1.0; expected2.data[1] = 0.0; expected2.data[2] = 0.0;
    assert(vectors_close(&eig2, &expected2, 1e-3));

    /* Test 3: Pauli X matrix [[0,1],[1,0]] */
    Matrix X;
    matrix_create(&X, 2, 2);
    double X_vals[4] = {0,1, 1,0};
    set_matrix(&X, X_vals);
    Vector eig3;
    vector_create(&eig3, 2);
    power_iteration(&X, &eig3, 1e-6, 1000);
    double val3 = eigenvalue(&X, &eig3);
    assert(fabs(val3 - 1.0) < 1e-4);
    Vector expected3;
    vector_create(&expected3, 2);
    double norm = sqrt(0.5);
    expected3.data[0] = norm; expected3.data[1] = norm;
    assert(vectors_close(&eig3, &expected3, 1e-3));

    /* Test 4: Zero matrix */
    Matrix Z;
    matrix_create(&Z, 2, 2);
    double Z_vals[4] = {0,0, 0,0};
    set_matrix(&Z, Z_vals);
    Vector eig4;
    vector_create(&eig4, 2);
    bool converged = power_iteration(&Z, &eig4, 1e-6, 1000);
    assert(converged);
    /* Any vector is eigenvector; check norm */
    assert(fabs(vector_norm(&eig4) - 1.0) < 1e-6);

    /* Clean up */
    vector_destroy(&eig1);
    vector_destroy(&eig2);
    vector_destroy(&eig3);
    vector_destroy(&eig4);
    vector_destroy(&expected2);
    vector_destroy(&expected3);
    matrix_destroy(&I);
    matrix_destroy(&D);
    matrix_destroy(&X);
    matrix_destroy(&Z);

    printf("All tests passed.\n");
    return 0;
}
