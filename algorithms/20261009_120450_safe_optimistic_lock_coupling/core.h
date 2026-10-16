#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>

#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Initialise an empty list. Returns a pointer to the new list or NULL on OOM. */
List *list_create(void);

/** Destroy a list and free all associated memory. */
void list_destroy(List *list);

/** Search for a key. If found, *out_value receives the associated value and true is returned. */
bool list_search(const List *list, int key, int *out_value);

/** Insert a key/value pair. Returns true on success, false if the key already exists or OOM. */
bool list_insert(List *list, int key, int value);

/** Delete a key. Returns true if the key was present and removed, false otherwise. */
bool list_delete(List *list, int key);

#ifdef __cplusplus
}
#endif
