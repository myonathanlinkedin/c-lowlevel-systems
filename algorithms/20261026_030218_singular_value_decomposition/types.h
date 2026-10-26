#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    size_t rows;
    size_t cols;
    double *data; /* row-major order */
} Matrix;

typedef struct {
    Matrix U;      /* m x k */
    double *S;     /* k */
    Matrix Vt;     /* k x n */
    size_t rank;   /* k */
} SVDResult;
