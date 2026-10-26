#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "core.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

static void test_identity(void) {
    Matrix A = matrix_create(3, 3);
    for (size_t i = 0; i < 3; ++i) {
        A.data[i * 3 + i] = 1.0;
    }
    SVDResult res;
    assert(svd_lowrank(&A, 3, &res));
    /* Check singular values close to 1 */
    for (size_t i = 0; i < 3; ++i) {
        assert(fabs(res.S[i] - 1.0) < 1e-9);
    }
    /* Check U and Vt are identity */
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            double u = (i == j) ? 1.0 : 0.0;
            double vt = (i == j) ? 1.0 : 0.0;
            assert(fabs(res.U.data[i * 3 + j] - u) < 1e-9);
            assert(fabs(res.Vt.data[i * 3 + j] - vt) < 1e-9);
        }
    }
    /* Reconstruction error */
    Matrix U_S = matrix_create(3, 3);
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            U_S.data[i * 3 + j] = res.U.data[i * 3 + j] * res.S[j];
        }
    }
    Matrix approx = matrix_multiply(&U_S, &res.Vt);
    double err = matrix_norm_frobenius(&approx);
    assert(err < 1e-9);
    /* Clean up */
    matrix_destroy(&A);
    matrix_destroy(&U_S);
    matrix_destroy(&approx);
    svd_lowrank_destroy(&res);
}

static void test_known_matrix(void) {
    Matrix A = matrix_create(2, 2);
    A.data[0] = 3.0; A.data[1] = 1.0;
    A.data[2] = 1.0; A.data[3] = 3.0;
    SVDResult res;
    assert(svd_lowrank(&A, 2, &res));
    /* Expected singular values: sqrt(4)=2, sqrt(2)=1.414213562 */
    assert(fabs(res.S[0] - 2.0) < 1e-9);
    assert(fabs(res.S[1] - sqrt(2.0)) < 1e-9);
    /* Reconstruct */
    Matrix U_S = matrix_create(2, 2);
    for (size_t i = 0; i < 2; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            U_S.data[i * 2 + j] = res.U.data[i * 2 + j] * res.S[j];
        }
    }
    Matrix approx = matrix_multiply(&U_S, &res.Vt);
    double err = matrix_norm_frobenius(&approx);
    assert(err < 1e-9);
    /* Clean up */
    matrix_destroy(&A);
    matrix_destroy(&U_S);
    matrix_destroy(&approx);
    svd_lowrank_destroy(&res);
}

static void test_low_rank_approx(void) {
    Matrix A = matrix_create(2, 2);
    A.data[0] = 1.0; A.data[1] = 2.0;
    A.data[2] = 3.0; A.data[3] = 4.0;
    SVDResult res;
    assert(svd_lowrank(&A, 1, &res));
    /* Reconstruct rank-1 approximation */
    Matrix U_S = matrix_create(2, 1);
    for (size_t i = 0; i < 2; ++i) {
        U_S.data[i] = res.U.data[i] * res.S[0];
    }
    Matrix approx = matrix_multiply(&U_S, &res.Vt);
    double err = matrix_norm_frobenius(&approx);
    /* Expected error: sqrt( (1-1)^2 + (2-2)^2 + (3-3)^2 + (4-4)^2 ) = 0 */
    /* But rank-1 approximation of full rank matrix will not be exact */
    assert(err > 0.0 && err < 1.0);
    /* Clean up */
    matrix_destroy(&A);
    matrix_destroy(&U_S);
    matrix_destroy(&approx);
    svd_lowrank_destroy(&res);
}

static void test_zero_matrix(void) {
    Matrix A = matrix_create(3, 3);
    SVDResult res;
    assert(svd_lowrank(&A, 2, &res));
    for (size_t i = 0; i < 2; ++i) {
        assert(fabs(res.S[i]) < 1e-12);
    }
    matrix_destroy(&A);
    svd_lowrank_destroy(&res);
}

static void test_rectangular(void) {
    Matrix A = matrix_create(3, 2);
    A.data[0] = 1.0; A.data[1] = 0.0;
    A.data[2] = 0.0; A.data[3] = 1.0;
    A.data[4] = 1.0; A.data[5] = 1.0;
    SVDResult res;
    assert(svd_lowrank(&A, 2, &res));
    /* Reconstruct */
    Matrix U_S = matrix_create(3, 2);
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            U_S.data[i * 2 + j] = res.U.data[i * 2 + j] * res.S[j];
        }
    }
    Matrix approx = matrix_multiply(&U_S, &res.Vt);
    double err = matrix_norm_frobenius(&approx);
    assert(err < 1e-9);
    matrix_destroy(&A);
    matrix_destroy(&U_S);
    matrix_destroy(&approx);
    svd_lowrank_destroy(&res);
}

int main(void) {
    test_identity();
    test_known_matrix();
    test_low_rank_approx();
    test_zero_matrix();
    test_rectangular();
    printf("All tests passed.\n");
    return 0;
}
