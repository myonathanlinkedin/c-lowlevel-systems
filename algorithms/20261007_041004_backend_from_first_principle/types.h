#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include <stddef.h>
#include <stdbool.h>

typedef struct Backend {
    size_t capacity;   // total slots
    size_t count;      // number of stored entries
    char **keys;       // array of key pointers (NULL = empty, (char*)1 = tombstone)
    int *values;       // parallel array of values
} Backend;
