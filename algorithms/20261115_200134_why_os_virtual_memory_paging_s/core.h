#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include "types.h"

void pager_init(struct Pager *p, size_t num_frames);
void pager_destroy(struct Pager *p);
void pager_access(struct Pager *p, uint64_t page_id);
size_t pager_get_faults(const struct Pager *p);
size_t pager_get_hits(const struct Pager *p);
size_t pager_get_capacity(const struct Pager *p);
size_t pager_get_loaded(const struct Pager *p);
