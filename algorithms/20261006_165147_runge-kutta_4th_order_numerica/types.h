#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stddef.h>

typedef void (*ode_func_t)(double t, const double *y, double *dy, void *params);
