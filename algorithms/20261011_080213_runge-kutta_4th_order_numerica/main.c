#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>

/* Forward declarations from engine.c */
rk4_integrator_t *rk4_create(size_t dim, double step, ode_func_t f, void *params);
void rk4_destroy(rk4_integrator_t *inst);
bool rk4_step(const rk4_integrator_t *inst, double t, const double *y, double *y_out);

/* ------------------- Test 1: dy/dt = -y ------------------- */
static void ode_exp_decay(double t, const double *y, double *dy, void *params) {
    (void)t; (void)params;
    dy[0] = -y[0];
}

/* ------------------- Test 2: Simple Harmonic Oscillator ------------------- */
static void ode_harmonic(double t, const double *y, double *dy, void *params) {
    (void)t; (void)params;
    /* y[0] = x (position), y[1] = v (velocity) */
    dy[0] = y[1];
    dy[1] = -y[0];
}

/* Utility: compare two doubles within tolerance */
static bool approx_equal(double a, double b, double tol) {
    return fabs(a - b) <= tol;
}

/* Run unit tests; aborts on failure via assert. */
int main(void) {
    /* ---------- Test 1: Exponential decay ---------- */
    {
        const double step = 0.1;
        const double t_end = 1.0;
        const double y0 = 1.0;
        const double tol = 1e-4;

        rk4_integrator_t *intg = rk4_create(1, step, ode_exp_decay, NULL);
        assert(intg != NULL);

        double t = 0.0;
        double y[1] = { y0 };
        double y_next[1];

        while (t < t_end - 1e-12) {
            bool ok = rk4_step(intg, t, y, y_next);
            assert(ok);
            t += step;
            y[0] = y_next[0];
        }

        double exact = y0 * exp(-t_end);
        assert(approx_equal(y[0], exact, tol));

        rk4_destroy(intg);
    }

    /* ---------- Test 2: Harmonic oscillator (period = 2π) ---------- */
    {
        const double step = 0.01;
        const double t_end = 2.0 * M_PI;   /* One full period */
        const double tol = 1e-3;

        rk4_integrator_t *intg = rk4_create(2, step, ode_harmonic, NULL);
        assert(intg != NULL);

        double t = 0.0;
        double y[2] = { 1.0, 0.0 };   /* Initial condition: x=1, v=0 */
        double y_next[2];

        while (t < t_end - 1e-12) {
            bool ok = rk4_step(intg, t, y, y_next);
            assert(ok);
            t += step;
            y[0] = y_next[0];
            y[1] = y_next[1];
        }

        /* After one period, state should return to initial condition */
        assert(approx_equal(y[0], 1.0, tol));
        assert(approx_equal(y[1], 0.0, tol));

        rk4_destroy(intg);
    }

    /* ---------- Edge Cases ---------- */
    {
        /* Zero dimension should be rejected */
        rk4_integrator_t *bad = rk4_create(0, 0.1, ode_exp_decay, NULL);
        assert(bad == NULL);
    }

    printf("All RK4 unit tests passed.\n");
    return 0;
}
