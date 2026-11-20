#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include <stddef.h>
#include <stdatomic.h>

typedef struct rc_string {
    char *data;
    atomic_uint refcount;
} rc_string;

size_t rc_string_alloc_count(void);
