#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>

static void allocate_temp(size_t n, double **k1, double **k2,
                          double **k3, double **k4, double **y_temp) {
    *k1 = (double *)malloc(n * sizeof(double));
    *k2 = (double *)malloc(n * sizeof(double));
    *k3 = (double *)malloc(n * sizeof(double));
    *k4 = (double *)malloc(n * sizeof(double));
    *y_temp = (double *)malloc(n * sizeof(double));
}

static void free_temp(double *k1, double *k2, double *k3,
                      double *k4, double *y_temp) {
    free(k1);
    free(k2);
    free(k3);
    free(k4);
    free(y_temp);
}

void rk4_step(double t, const double *y, double h, size_t n,
              ode_func_t f, void *params, double *y_next) {
    double *k1, *k2, *k3, *k4, *y_temp;
    allocate_temp(n, &k1, &k2, &k3, &k4, &y_temp);

    f(t, y, k1, params);

    for (size_t i = 0; i < n; ++i)
        y_temp[i] = y[i] + h * 0.5 * k1[i];
    f(t + h * 0.5, y_temp, k2, params);

    for (size_t i = 0; i < n; ++i)
        y_temp[i] = y[i] + h * 0.5 * k2[i];
    f(t + h * 0.5, y_temp, k3, params);

    for (size_t i = 0; i < n; ++i)
        y_temp[i] = y[i] + h * k3[i];
    f(t + h, y_temp, k4, params);

    for (size_t i = 0; i < n; ++i)
        y_next[i] = y[i] + (h / 6.0) * (k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]);

    free_temp(k1, k2, k3, k4, y_temp);
}

void rk4_integrate(double t0, double t1, double h, const double *y0,
                   size_t n, ode_func_t f, void *params,
                   double *t_out, double *y_out) {
    size_t steps = (size_t)((t1 - t0) / h) + 1;
    double *y_curr = (double *)malloc(n * sizeof(double));
    memcpy(y_curr, y0, n * sizeof(double));

    for (size_t i = 0; i < steps; ++i) {
        double t = t0 + i * h;
        t_out[i] = t;
        memcpy(&y_out[i * n], y_curr, n * sizeof(double));
        if (i < steps - 1)
            rk4_step(t, y_curr, h, n, f, params, y_curr);
    }
    free(y_curr);
}
