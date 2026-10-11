#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    void* data;
    size_t size;
    size_t immut_count;
    bool mut_active;
} BorrowedObject;
