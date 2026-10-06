#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#include "types.h"
#include "core.h"
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <assert.h>

static void ode_exp(double t, const double *y, double *dy, void *params) {
    (void)t; (void)params;
    dy[0] = y[0];
}

static void ode_decay(double t, const double *y, double *dy, void *params) {
    (void)t; (void)params;
    dy[0] = -y[0];
}

static void ode_harmonic(double t, const double *y, double *dy, void *params) {
    (void)t;
    double omega = *(double *)params;
    dy[0] = y[1];
    dy[1] = -omega * omega * y[0];
}

static void test_exponential(void) {
    double y0 = 1.0;
    double t0 = 0.0, t1 = 1.0;
    double h = 0.01;
    size_t n = 1;
    double *t_out = (double *)malloc(((size_t)((t1 - t0) / h) + 1) * sizeof(double));
    double *y_out = (double *)malloc(((size_t)((t1 - t0) / h) + 1) * n * sizeof(double));

    rk4_integrate(t0, t1, h, &y0, n, ode_exp, NULL, t_out, y_out);

    double y_num = y_out[((size_t)((t1 - t0) / h)) * n];
    double y_exact = exp(t1);
    assert(fabs(y_num - y_exact) < 1e-4);

    free(t_out);
    free(y_out);
}

static void test_decay(void) {
    double y0 = 1.0;
    double t0 = 0.0, t1 = 1.0;
    double h = 0.01;
    size_t n = 1;
    double *t_out = (double *)malloc(((size_t)((t1 - t0) / h) + 1) * sizeof(double));
    double *y_out = (double *)malloc(((size_t)((t1 - t0) / h) + 1) * n * sizeof(double));

    rk4_integrate(t0, t1, h, &y0, n, ode_decay, NULL, t_out, y_out);

    double y_num = y_out[((size_t)((t1 - t0) / h)) * n];
    double y_exact = exp(-t1);
    assert(fabs(y_num - y_exact) < 1e-4);

    free(t_out);
    free(y_out);
}

static void test_harmonic(void) {
    double y0[2] = {1.0, 0.0};
    double t0 = 0.0, t1 = 2 * M_PI;
    double h = 0.001;
    size_t n = 2;
    double omega = 1.0;
    double *t_out = (double *)malloc(((size_t)((t1 - t0) / h) + 1) * sizeof(double));
    double *y_out = (double *)malloc(((size_t)((t1 - t0) / h) + 1) * n * sizeof(double));

    rk4_integrate(t0, t1, h, y0, n, ode_harmonic, &omega, t_out, y_out);

    double y_num = y_out[((size_t)((t1 - t0) / h)) * n];
    double y_exact = 1.0; // cos(2π) = 1
    assert(fabs(y_num - y_exact) < 1e-3);

    free(t_out);
    free(y_out);
}

static void benchmark(void) {
    double y0 = 1.0;
    double t0 = 0.0, t1 = 10.0;
    double h = 0.001;
    size_t n = 1;
    double *t_out = (double *)malloc(((size_t)((t1 - t0) / h) + 1) * sizeof(double));
    double *y_out = (double *)malloc(((size_t)((t1 - t0) / h) + 1) * n * sizeof(double));

    clock_t start = clock();
    rk4_integrate(t0, t1, h, &y0, n, ode_exp, NULL, t_out, y_out);
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Benchmark: %f seconds for %zu steps\n", elapsed,
           (size_t)((t1 - t0) / h) + 1);

    free(t_out);
    free(y_out);
}

int main(void) {
    test_exponential();
    test_decay();
    test_harmonic();
    benchmark();
    printf("All tests passed.\n");
    return 0;
}
