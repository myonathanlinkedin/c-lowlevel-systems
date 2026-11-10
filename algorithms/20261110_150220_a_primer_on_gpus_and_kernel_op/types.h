#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef void (*gpu_kernel_t)(size_t global_id, void *args);

typedef struct {
    size_t gridDimX;
    size_t blockDimX;
} gpu_grid_t;

// Arguments structure for vector addition kernel
typedef struct {
    const float *a;
    const float *b;
    float *c;
    size_t n;
} vec_add_args_t;
