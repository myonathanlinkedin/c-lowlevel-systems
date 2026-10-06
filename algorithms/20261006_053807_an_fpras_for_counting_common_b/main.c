#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#include "types.h"
#include <stdio.h>
#include <assert.h>
#include <time.h>

int main(void) {
    srand((unsigned)time(NULL));

    // Test 1: U_{3,5} vs U_{3,5} -> 10 common bases
    Matroid m1 = {5, 3, uniform_is_independent};
    Matroid m2 = {5, 3, uniform_is_independent};
    FPRASParams params = {10000, 0.01};
    ApproxResult res;
    approximate_common_bases(&m1, &m2, &params, &res);
    printf("Test 1 Estimate: %.2f ± %.2f\n", res.estimate, res.error);
    assert(fabs(res.estimate - 10.0) <= 0.05 * 10.0);

    // Test 2: U_{3,5} vs U_{4,5} -> 0 common bases
    Matroid m3 = {5, 4, uniform_is_independent};
    approximate_common_bases(&m1, &m3, &params, &res);
    printf("Test 2 Estimate: %.2f ± %.2f\n", res.estimate, res.error);
    assert(res.estimate <= 0.05);

    // Test 3: Rank 0 matroids
    Matroid m4 = {5, 0, uniform_is_independent};
    Matroid m5 = {5, 0, uniform_is_independent};
    approximate_common_bases(&m4, &m5, &params, &res);
    printf("Test 3 Estimate: %.2f ± %.2f\n", res.estimate, res.error);
    assert(fabs(res.estimate - 1.0) <= 0.05);

    // Test 4: Rank n matroids
    Matroid m6 = {5, 5, uniform_is_independent};
    Matroid m7 = {5, 5, uniform_is_independent};
    approximate_common_bases(&m6, &m7, &params, &res);
    printf("Test 4 Estimate: %.2f ± %.2f\n", res.estimate, res.error);
    assert(fabs(res.estimate - 1.0) <= 0.05);

    printf("All tests passed.\n");
    return 0;
}
