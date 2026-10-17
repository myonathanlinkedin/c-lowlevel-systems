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

/**
 * @brief Initialise a ring buffer.
 *
 * @param rb        Pointer to an uninitialised ring_buffer_t.
 * @param capacity  Desired capacity (will be rounded up to next power of two).
 * @return true on success, false on allocation failure or zero capacity.
 */
bool ring_init(ring_buffer_t *rb, size_t capacity);

/**
 * @brief Release resources held by a ring buffer.
 *
 * @param rb Pointer to an initialised ring_buffer_t.
 */
void ring_free(ring_buffer_t *rb);

/**
 * @brief Push a value into the buffer (single‑producer safe).
 *
 * @param rb    Pointer to an initialised ring_buffer_t.
 * @param value Value to store.
 * @return true if the value was stored, false if the buffer is full.
 */
bool ring_push(ring_buffer_t *rb, int value);

/**
 * @brief Pop a value from the buffer (single‑consumer safe).
 *
 * @param rb    Pointer to an initialised ring_buffer_t.
 * @param value Output location for the popped value.
 * @return true if a value was retrieved, false if the buffer is empty.
 */
bool ring_pop(ring_buffer_t *rb, int *value);

#ifdef __cplusplus
}
#endif
