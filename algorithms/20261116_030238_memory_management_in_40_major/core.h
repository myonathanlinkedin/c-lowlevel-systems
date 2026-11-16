#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#pragma once
#include "types.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *tracked_malloc(size_t size, const char *file, int line);
void tracked_free(void *ptr, const char *file, int line);
size_t mem_total_allocated(void);
size_t mem_current_allocated(void);
size_t mem_peak_allocated(void);
size_t mem_allocation_count(void);
void mem_report(void);
void mem_reset(void);

#define tmalloc(s) tracked_malloc((s), __FILE__, __LINE__)
#define tfree(p)   tracked_free((p), __FILE__, __LINE__)

#ifdef __cplusplus
}
#endif
