#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t u;
    uint32_t v;
    int32_t weight;
} Edge;

typedef struct {
    uint32_t *parent;
    uint32_t *rank;
    size_t size;
} DSU;
