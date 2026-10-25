#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

static bool float_eq(float a, float b, float eps) {
    return fabsf(a - b) <= eps;
}

int main(void) {
    // Test 1: creation and zero‑length handling
    Vector *v0 = vector_create(0);
    assert(v0 != NULL);
    assert(v0->len == 0);
    assert(v0->data == NULL);
    vector_free(v0);

    // Test 2: fill and basic properties
    const size_t N = 8;
    Vector *v1 = vector_create(N);
    vector_fill(v1, 1.5f);
    for (size_t i = 0; i < N; ++i) {
        assert(float_eq(v1->data[i], 1.5f, 1e-6f));
    }

    // Test 3: scaling
    Vector *v2 = vector_create(N);
    vector_scale(v1, 2.0f, v2);
    for (size_t i = 0; i < N; ++i) {
        assert(float_eq(v2->data[i], 3.0f, 1e-6f));
    }

    // Test 4: addition
    Vector *v3 = vector_create(N);
    vector_fill(v3, -0.5f);
    Vector *v_add = vector_create(N);
    vector_add(v2, v3, v_add);
    for (size_t i = 0; i < N; ++i) {
        assert(float_eq(v_add->data[i], 2.5f, 1e-6f));
    }

    // Test 5: dot product
    float dot_res = 0.0f;
    vector_dot(v2, v3, &dot_res);
    // Expected: Σ (3.0 * -0.5) = N * -1.5
    float expected_dot = (float)N * -1.5f;
    assert(float_eq(dot_res, expected_dot, 1e-5f));

    // Test 6: mismatched length assertions (should abort)
    // Uncomment to verify that assertions fire as expected.
    // Vector *v_bad = vector_create(N + 1);
    // vector_add(v1, v_bad, v_add); // should assert

    // Clean up
    vector_free(v1);
    vector_free(v2);
    vector_free(v3);
    vector_free(v_add);

    // Benchmark: simple large vector addition (deterministic, no timing required)
    const size_t M = 1024 * 1024; // 1M elements
    Vector *a = vector_create(M);
    Vector *b = vector_create(M);
    Vector *c = vector_create(M);
    vector_fill(a, 1.0f);
    vector_fill(b, 2.0f);
    vector_add(a, b, c);
    for (size_t i = 0; i < M; ++i) {
        assert(float_eq(c->data[i], 3.0f, 1e-6f));
    }
    vector_free(a);
    vector_free(b);
    vector_free(c);

    printf("All unit tests passed.\n");
    return 0;
}
