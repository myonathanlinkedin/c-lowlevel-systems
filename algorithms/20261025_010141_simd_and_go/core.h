#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#pragma once
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Allocate a vector of given length. Elements are uninitialized.
Vector* vector_create(size_t len);

// Release vector memory.
void vector_free(Vector *v);

// Fill vector with a constant value.
void vector_fill(Vector *v, float value);

// Element‑wise addition: result = a + b. All vectors must have same length.
void vector_add(const Vector *a, const Vector *b, Vector *result);

// Scale vector by scalar: result = a * scalar.
void vector_scale(const Vector *a, float scalar, Vector *result);

// Dot product: *out = Σ a[i] * b[i]. Vectors must have same length.
void vector_dot(const Vector *a, const Vector *b, float *out);

#ifdef __cplusplus
}
#endif
