
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Simple key‑value pair used by the temporal queue.
typedef struct {
    uint64_t timestamp;   // monotonic time key
    int      value;       // associated payload
} TemporalItem;

// Forward declaration of the queue structure.
typedef struct TemporalQueue TemporalQueue;
