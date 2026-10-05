#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

/* Declarations from core.c */
typedef struct {
    const char *name;
    double elapsed_sec;
    double ops_per_sec;
} BenchmarkResult;

extern void core_run_matrix_benchmark(size_t n, size_t iterations, BenchmarkResult *out);
extern bool core_test_matrix_correctness(void);
extern void benchmark_report(const BenchmarkResult *results, size_t count);

static void test_correctness(void) {
    printf("[TEST] Matrix multiplication correctness ... ");
    bool ok = core_test_matrix_correctness();
    printf("%s\n", ok ? "PASS" : "FAIL");
    assert(ok && "Matrix multiplication correctness test failed");
}

static void benchmark_matrix(void) {
    const size_t n = 128;          /* matrix dimension */
    const size_t iters = 5;        /* repeat count for timing */
    BenchmarkResult result;
    core_run_matrix_benchmark(n, iters, &result);
    benchmark_report(&result, 1);
}

int main(void) {
    printf("=== D2K-Bench Prototype ===\n");
    test_correctness();
    benchmark_matrix();
    return 0;
}
