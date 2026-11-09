#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#pragma once
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Create a new empty min‑heap. Returns NULL on allocation failure. */
MinHeap* heap_create(void);

/* Release all resources associated with the heap. */
void heap_destroy(MinHeap* heap);

/* Query whether the heap contains no elements. */
bool heap_is_empty(const MinHeap* heap);

/* Return the number of elements stored in the heap. */
size_t heap_size(const MinHeap* heap);

/* Insert a value into the heap. Returns false on allocation failure. */
bool heap_push(MinHeap* heap, int value);

/* Retrieve the smallest element without removing it.
   Returns false if the heap is empty. */
bool heap_peek(const MinHeap* heap, int* out);

/* Remove the smallest element and store it in *out.
   Returns false if the heap is empty. */
bool heap_pop(MinHeap* heap, int* out);

#ifdef __cplusplus
}
#endif
