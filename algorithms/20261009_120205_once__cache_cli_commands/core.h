#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#pragma once
#include "types.h"

/* Intern a command string.
 * Returns a pointer to a stored copy of the string.
 * Subsequent calls with an equal string return the same pointer.
 * Returns NULL if input is NULL or on allocation failure.
 */
const char *cache_command(const char *cmd);

/* Release all cached strings and internal structures.
 * After calling, the cache can be used again.
 */
void cache_free(void);
