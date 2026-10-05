#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <assert.h>

typedef struct {
    const char *name;
    double elapsed_sec;
    double ops_per_sec;
} BenchmarkResult;

static double get_time_sec(void) {
#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
    }
#endif
    return (double)clock() / (double)CLOCKS_PER_SEC;
}

static void run_benchmark(const char *name,
                          void (*func)(void *),
                          void *arg,
                          size_t iterations,
                          BenchmarkResult *out) {
    assert(func != NULL && out != NULL);
    double start = get_time_sec();
    for (size_t i = 0; i < iterations; ++i) {
        func(arg);
    }
    double end = get_time_sec();
    out->name = name;
    out->elapsed_sec = end - start;
    out->ops_per_sec = (iterations > 0 && out->elapsed_sec > 0.0) ?
                       (double)iterations / out->elapsed_sec : 0.0;
}

static void benchmark_report(const BenchmarkResult *results, size_t count) {
    printf("=== Benchmark Report ===\n");
    for (size_t i = 0; i < count; ++i) {
        const BenchmarkResult *r = &results[i];
        printf("%-30s : %8.6f sec, %12.2f ops/sec\n",
               r->name, r->elapsed_sec, r->ops_per_sec);
    }
    printf("========================\n");
}

/* ---------- Matrix Multiplication ---------- */
typedef struct {
    size_t n;      /* dimension (n x n) */
    double *A;     /* left matrix */
    double *B;     /* right matrix */
    double *C;     /* result matrix */
} MatrixMulArgs;

/* Naïve O(n^3) multiplication: C = A * B */
static void matrix_mul(void *arg) {
    MatrixMulArgs *m = (MatrixMulArgs *)arg;
    size_t n = m->n;
    double *A = m->A;
    double *B = m->B;
    double *C = m->C;

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < n; ++k) {
                sum += A[i * n + k] * B[k * n + j];
            }
            C[i * n + j] = sum;
        }
    }
}

/* Helper: allocate and zero-initialize a matrix */
static double *alloc_matrix(size_t n) {
    double *m = (double *)aligned_alloc(64, n * n * sizeof(double));
    if (!m) {
        perror("aligned_alloc");
        exit(EXIT_FAILURE);
    }
    memset(m, 0, n * n * sizeof(double));
    return m;
}

/* Helper: fill matrix with deterministic pseudo‑random data */
static void fill_matrix(double *m, size_t n, unsigned seed) {
    srand(seed);
    for (size_t i = 0; i < n * n; ++i) {
        m[i] = (double)(rand() % 100) / 10.0;  /* values in [0,9.9] */
    }
}

/* Helper: compare two matrices within tolerance */
static bool compare_matrices(const double *a, const double *b, size_t n, double eps) {
    for (size_t i = 0; i < n * n; ++i) {
        double diff = a[i] - b[i];
        if (diff < -eps || diff > eps) {
            return false;
        }
    }
    return true;
}

/* Public API */
extern void core_run_matrix_benchmark(size_t n, size_t iterations, BenchmarkResult *out);
extern bool core_test_matrix_correctness(void);

void core_run_matrix_benchmark(size_t n, size_t iterations, BenchmarkResult *out) {
    MatrixMulArgs args;
    args.n = n;
    args.A = alloc_matrix(n);
    args.B = alloc_matrix(n);
    args.C = alloc_matrix(n);
    fill_matrix(args.A, n, 1);
    fill_matrix(args.B, n, 2);
    run_benchmark("matrix_mul", matrix_mul, &args, iterations, out);
    free(args.A);
    free(args.B);
    free(args.C);
}

/* Verify correctness against a simple reference implementation (same as matrix_mul) */
bool core_test_matrix_correctness(void) {
    const size_t n = 4;
    MatrixMulArgs args;
    args.n = n;
    args.A = alloc_matrix(n);
    args.B = alloc_matrix(n);
    args.C = alloc_matrix(n);
    fill_matrix(args.A, n, 42);
    fill_matrix(args.B, n, 84);
    matrix_mul(&args);  /* compute reference result in C */

    double *C_ref = alloc_matrix(n);
    memcpy(C_ref, args.C, n * n * sizeof(double));

    /* Zero C and recompute using the same function (acts as test) */
    memset(args.C, 0, n * n * sizeof(double));
    matrix_mul(&args);

    bool ok = compare_matrices(args.C, C_ref, n, 1e-9);
    free(args.A);
    free(args.B);
    free(args.C);
    free(C_ref);
    return ok;
}
