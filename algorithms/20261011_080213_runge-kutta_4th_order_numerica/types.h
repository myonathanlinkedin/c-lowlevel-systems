#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once

#include <stddef.h>
#include <stdbool.h>

/* ODE system function type.
 * Parameters:
 *   t      - current time
 *   y      - pointer to input state vector (length = dim)
 *   dy     - pointer to output derivative vector (length = dim)
 *   params - user-provided parameters (may be NULL)
 */
typedef void (*ode_func_t)(double t, const double *y, double *dy, void *params);

/* Structure describing an RK4 integrator instance. */
typedef struct {
    size_t dim;            /* Dimension of the state vector */
    double step;           /* Fixed step size */
    ode_func_t f;          /* ODE system function */
    void *params;          /* Optional user parameters passed to f */
} rk4_integrator_t;
