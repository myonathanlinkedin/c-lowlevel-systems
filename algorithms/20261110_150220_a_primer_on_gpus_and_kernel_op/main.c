#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "types.h"
#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#define EPSILON 1e-6f

static void test_zero_length(void)
{
    float *c = NULL;
    gpu_vector_add(NULL, NULL, c, 0);
    // No crash, nothing to assert.
}

static void test_single_element(void)
{
    float a = 1.23f, b = 4.56f, c = 0.0f;
    gpu_vector_add(&a, &b, &c, 1);
    assert(fabsf(c - (a + b)) < EPSILON);
}

static void test_multiple_of_block(void)
{
    const size_t N = 1024; // 1024 = 4 * 256
    float *a = (float *)malloc(N * sizeof(float));
    float *b = (float *)malloc(N * sizeof(float));
    float *c = (float *)malloc(N * sizeof(float));
    float *ref = (float *)malloc(N * sizeof(float));

    for (size_t i = 0; i < N; ++i) {
        a[i] = (float)i * 0.5f;
        b[i] = (float)i * -0.25f;
        ref[i] = a[i] + b[i];
    }

    gpu_vector_add(a, b, c, N);

    for (size_t i = 0; i < N; ++i) {
        assert(fabsf(c[i] - ref[i]) < EPSILON);
    }

    free(a);
    free(b);
    free(c);
    free(ref);
}

static void test_non_multiple_of_block(void)
{
    const size_t N = 1000; // Not a multiple of 256
    float *a = (float *)malloc(N * sizeof(float));
    float *b = (float *)malloc(N * sizeof(float));
    float *c = (float *)malloc(N * sizeof(float));
    float *ref = (float *)malloc(N * sizeof(float));

    for (size_t i = 0; i < N; ++i) {
        a[i] = (float)(i % 7) + 0.1f;
        b[i] = (float)(i % 5) - 0.2f;
        ref[i] = a[i] + b[i];
    }

    gpu_vector_add(a, b, c, N);

    for (size_t i = 0; i < N; ++i) {
        assert(fabsf(c[i] - ref[i]) < EPSILON);
    }

    free(a);
    free(b);
    free(c);
    free(ref);
}

static void test_large_vector(void)
{
    const size_t N = 500000; // Large but still fits in memory quickly
    float *a = (float *)malloc(N * sizeof(float));
    float *b = (float *)malloc(N * sizeof(float));
    float *c = (float *)malloc(N * sizeof(float));

    for (size_t i = 0; i < N; ++i) {
        a[i] = (float)i * 1.001f;
        b[i] = (float)i * -0.999f;
    }

    gpu_vector_add(a, b, c, N);

    for (size_t i = 0; i < N; ++i) {
        float expected = a[i] + b[i];
        assert(fabsf(c[i] - expected) < EPSILON);
    }

    free(a);
    free(b);
    free(c);
}

int main(void)
{
    test_zero_length();
    test_single_element();
    test_multiple_of_block();
    test_non_multiple_of_block();
    test_large_vector();

    printf("All GPU vector‑add tests passed.\n");
    return 0;
}
