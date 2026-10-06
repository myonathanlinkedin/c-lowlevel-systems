#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    MEM_OK = 0,
    MEM_ERR_ALLOC,
    MEM_ERR_BOUNDS,
    MEM_ERR_NULL
} MemStatus;

typedef struct {
    uint8_t* data;
    size_t capacity;
    size_t size;
} Stack;

typedef struct {
    uint8_t* data;
    size_t capacity;
    size_t size;
} Heap;
