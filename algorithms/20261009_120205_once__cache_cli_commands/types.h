
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stddef.h>
#include <stdbool.h>

typedef struct InternNode {
    char *str;
    struct InternNode *next;
} InternNode;
