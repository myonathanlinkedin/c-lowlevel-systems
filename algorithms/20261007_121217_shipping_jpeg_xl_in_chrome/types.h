#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t width;
    uint32_t height;
    bool has_alpha;
} JXLImageInfo;

typedef enum {
    JXL_OK = 0,
    JXL_ERR_INVALID_DATA = -1,
    JXL_ERR_INSUFFICIENT_SIZE = -2,
    JXL_ERR_UNSUPPORTED_FORMAT = -3
} JXLResult;
