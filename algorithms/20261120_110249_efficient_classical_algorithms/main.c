#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "types.h"
#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>

static void test_identity() {
    Matrix *m = matrix_alloc(3, 3);
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            matrix_set(m, i, j, (i == j) ? 1.0 : 0.0);
        }
    }
    double energy = matrix_ground_energy(m);
    assert(fabs(energy - 1.0) < 1e-8);
}
