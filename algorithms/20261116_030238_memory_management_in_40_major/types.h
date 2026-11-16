#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include <stddef.h>
#include <stdbool.h>

typedef struct Allocation {
    void *ptr;
    size_t size;
    const char *file;
    int line;
    struct Allocation *next;
} Allocation;
