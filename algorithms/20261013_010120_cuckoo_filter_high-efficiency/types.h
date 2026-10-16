#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/* Cuckoo Filter configuration constants */
#define CF_MAX_RELOCATIONS 500U   /* Maximum number of evictions during insert */

/* Public opaque type for the filter */
typedef struct CuckooFilter CuckooFilter;

/* API */
CuckooFilter *cf_create(size_t bucket_count,
                        size_t bucket_size,
                        uint8_t fingerprint_bits);
void cf_destroy(CuckooFilter *cf);

bool cf_insert(CuckooFilter *cf,
               const void *item,
               size_t len);
bool cf_contains(const CuckooFilter *cf,
                 const void *item,
                 size_t len);
bool cf_delete(CuckooFilter *cf,
               const void *item,
               size_t len);
