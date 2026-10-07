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

Backend* backend_create(size_t initial_capacity);
void backend_destroy(Backend *db);
bool backend_set(Backend *db, const char *key, int value);
bool backend_get(const Backend *db, const char *key, int *out_value);
bool backend_delete(Backend *db, const char *key);

#ifdef __cplusplus
}
#endif
