#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Public API: perform element‑wise addition of two vectors using a simulated GPU kernel.
void gpu_vector_add(const float *a, const float *b, float *c, size_t n);

#ifdef __cplusplus
}
#endif
