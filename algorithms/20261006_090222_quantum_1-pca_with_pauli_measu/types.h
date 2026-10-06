#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint64_t pca_result;
    uint64_t pauli_measurements;
} Quantum1PCA_t;
