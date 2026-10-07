#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t *factors; // invariant factors, sorted ascending
    size_t len;
} Group;

typedef struct {
    uint32_t *coords; // coordinates modulo each factor
    size_t len;
} Element;

typedef struct {
    Element *vectors; // array of basis vectors
    size_t len;
} Basis;
