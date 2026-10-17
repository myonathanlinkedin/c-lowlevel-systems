#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once

#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef uint64_t hll_t;

#define HLL_SIZE 16

// Define a type for HyperLogLog counter
typedef struct hll_counter_t {
    hll_t counters[HLL_SIZE];
} hll_counter_t;

#endif // TYPES_H
