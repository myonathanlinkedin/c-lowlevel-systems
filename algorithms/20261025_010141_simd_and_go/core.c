#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <assert.h>

Vector* vector_create(size_t len) {
    Vector *v = (Vector*)malloc(sizeof(Vector));
    assert(v != NULL);
    v->len = len;
    if (len > 0) {
        v->data = (float*)malloc(len * sizeof(float));
        assert(v->data != NULL);
    } else {
        v->data = NULL;
    }
    return v;
}

void vector_free(Vector *v) {
    if (v) {
        free(v->data);
        free(v);
    }
}

void vector_fill(Vector *v, float value) {
    assert(v != NULL);
    for (size_t i = 0; i < v->len; ++i) {
        v->data[i] = value;
    }
}

void vector_add(const Vector *a, const Vector *b, Vector *result) {
    assert(a && b && result);
    assert(a->len == b->len && a->len == result->len);
    for (size_t i = 0; i < a->len; ++i) {
        result->data[i] = a->data[i] + b->data[i];
    }
}

void vector_scale(const Vector *a, float scalar, Vector *result) {
    assert(a && result);
    assert(a->len == result->len);
    for (size_t i = 0; i < a->len; ++i) {
        result->data[i] = a->data[i] * scalar;
    }
}

void vector_dot(const Vector *a, const Vector *b, float *out) {
    assert(a && b && out);
    assert(a->len == b->len);
    float acc = 0.0f;
    for (size_t i = 0; i < a->len; ++i) {
        acc += a->data[i] * b->data[i];
    }
    *out = acc;
}
