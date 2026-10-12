#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#pragma once
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Create an empty temporal queue. Returns NULL on allocation failure.
TemporalQueue* tq_create(void);

// Destroy a temporal queue and free all resources. Safe to call with NULL.
void tq_destroy(TemporalQueue* q);

// Insert a new item. Returns true on success, false on allocation failure.
bool tq_push(TemporalQueue* q, uint64_t timestamp, int value);

// Retrieve and remove the earliest‑timestamp item.
// Returns true if an item was removed, false if the queue was empty.
bool tq_pop(TemporalQueue* q, uint64_t* out_timestamp, int* out_value);

// Look at the earliest‑timestamp item without removing it.
// Returns true if an item exists, false otherwise.
bool tq_peek(const TemporalQueue* q, uint64_t* out_timestamp, int* out_value);

// Remove all items whose timestamp is strictly less than cutoff.
void tq_purge_before(TemporalQueue* q, uint64_t cutoff);

// Return the number of items currently stored.
size_t tq_size(const TemporalQueue* q);

#ifdef __cplusplus
}
#endif
