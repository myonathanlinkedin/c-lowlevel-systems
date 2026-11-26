#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define TSP_MAX_N 20          // maximum number of cities supported
#define TSP_EPS   1e-9        // tolerance for floating‑point comparisons
