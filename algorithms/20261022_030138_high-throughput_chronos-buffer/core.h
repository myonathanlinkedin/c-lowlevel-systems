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
 * @brief Initialize a ring buffer.
 *
 * @param rb Pointer to ChronosRingBuffer structure (may be uninitialized).
 * @param capacity Desired capacity; must be a power of two and > 0.
 * @return true on success, false on failure (e.g., non‑power‑of‑two or allocation error).
 */
bool crb_init(ChronosRingBuffer *rb, size_t capacity);

/**
 * @brief Release resources held by the ring buffer.
 *
 * @param rb Pointer to an initialized ChronosRingBuffer.
 */
void crb_free(ChronosRingBuffer *rb);

/**
 * @brief Push an item into the buffer (single‑producer).
 *
 * @param rb Pointer to an initialized ChronosRingBuffer.
 * @param item Pointer to the item to store.
 * @return true if the item was stored, false if the buffer is full.
 */
bool crb_push(ChronosRingBuffer *rb, void *item);

/**
 * @brief Pop an item from the buffer (single‑consumer).
 *
 * @param rb Pointer to an initialized ChronosRingBuffer.
 * @param item_out Receives the pointer to the popped item.
 * @return true if an item was retrieved, false if the buffer is empty.
 */
bool crb_pop(ChronosRingBuffer *rb, void **item_out);

/**
 * @brief Query the number of stored elements.
 *
 * @param rb Pointer to an initialized ChronosRingBuffer.
 * @return Number of elements currently stored.
 */
size_t crb_size(const ChronosRingBuffer *rb);

/**
 * @brief Test whether the buffer is full.
 *
 * @param rb Pointer to an initialized ChronosRingBuffer.
 * @return true if full, false otherwise.
 */
bool crb_is_full(const ChronosRingBuffer *rb);

/**
 * @brief Test whether the buffer is empty.
 *
 * @param rb Pointer to an initialized ChronosRingBuffer.
 * @return true if empty, false otherwise.
 */
bool crb_is_empty(const ChronosRingBuffer *rb);

#ifdef __cplusplus
}
#endif
