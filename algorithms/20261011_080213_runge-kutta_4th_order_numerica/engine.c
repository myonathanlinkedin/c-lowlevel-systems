#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdlib.h>
#include <string.h>

/* Allocate a new RK4 integrator.
 * Returns NULL on allocation failure.
 */
rk4_integrator_t *rk4_create(size_t dim, double step, ode_func_t f, void *params) {
    if (dim == 0 || step <= 0.0 || f == NULL) {
        return NULL;
    }
    rk4_integrator_t *inst = (rk4_integrator_t *)malloc(sizeof(rk4_integrator_t));
    if (!inst) {
        return NULL;
    }
    inst->dim = dim;
    inst->step = step;
    inst->f = f;
    inst->params = params;
    return inst;
}

/* Destroy an RK4 integrator instance. */
void rk4_destroy(rk4_integrator_t *inst) {
    if (inst) {
        free(inst);
    }
}

/* Perform a single RK4 step.
 *   inst - pointer to a valid rk4_integrator_t
 *   t    - current time (input)
 *   y    - pointer to current state vector (length = dim)
 *   y_out- pointer to buffer where the new state will be stored (length = dim)
 *
 * The function does NOT modify the input state vector y.
 * Returns true on success, false on invalid arguments.
 */
bool rk4_step(const rk4_integrator_t *inst, double t, const double *y, double *y_out) {
    if (!inst || !y || !y_out) {
        return false;
    }
    size_t n = inst->dim;
    double h = inst->step;
    double *k1 = (double *)malloc(n * sizeof(double));
    double *k2 = (double *)malloc(n * sizeof(double));
    double *k3 = (double *)malloc(n * sizeof(double));
    double *k4 = (double *)malloc(n * sizeof(double));
    double *y_temp = (double *)malloc(n * sizeof(double));
    if (!k1 || !k2 || !k3 || !k4 || !y_temp) {
        free(k1); free(k2); free(k3); free(k4); free(y_temp);
        return false;
    }

    /* k1 = f(t, y) */
    inst->f(t, y, k1, inst->params);

    /* k2 = f(t + h/2, y + h/2 * k1) */
    for (size_t i = 0; i < n; ++i) {
        y_temp[i] = y[i] + (h * 0.5) * k1[i];
    }
    inst->f(t + h * 0.5, y_temp, k2, inst->params);

    /* k3 = f(t + h/2, y + h/2 * k2) */
    for (size_t i = 0; i < n; ++i) {
        y_temp[i] = y[i] + (h * 0.5) * k2[i];
    }
    inst->f(t + h * 0.5, y_temp, k3, inst->params);

    /* k4 = f(t + h, y + h * k3) */
    for (size_t i = 0; i < n; ++i) {
        y_temp[i] = y[i] + h * k3[i];
    }
    inst->f(t + h, y_temp, k4, inst->params);

    /* y_out = y + h/6 * (k1 + 2*k2 + 2*k3 + k4) */
    for (size_t i = 0; i < n; ++i) {
        y_out[i] = y[i] + (h / 6.0) * (k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]);
    }

    free(k1);
    free(k2);
    free(k3);
    free(k4);
    free(y_temp);
    return true;
}
