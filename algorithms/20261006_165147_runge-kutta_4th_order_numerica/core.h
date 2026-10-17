#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include "types.h"

void rk4_step(double t, const double *y, double h, size_t n,
              ode_func_t f, void *params, double *y_next);

void rk4_integrate(double t0, double t1, double h, const double *y0,
                   size_t n, ode_func_t f, void *params,
                   double *t_out, double *y_out);
