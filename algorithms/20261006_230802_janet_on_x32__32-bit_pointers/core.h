#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include "types.h"

void arena_init(CompactArena *arena, size_t capacity);
void arena_destroy(CompactArena *arena);
CompactPtr arena_alloc(CompactArena *arena, size_t size);
void *compact_ptr_deref(const CompactArena *arena, CompactPtr cp);
size_t arena_remaining(const CompactArena *arena);
