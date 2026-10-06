#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void arena_init(CompactArena *arena, size_t capacity) {
    arena->base = (uint8_t *)malloc(capacity);
    assert(arena->base != NULL);
    arena->capacity = capacity;
    arena->used = 0;
}

void arena_destroy(CompactArena *arena) {
    free(arena->base);
    arena->base = NULL;
    arena->capacity = arena->used = 0;
}

CompactPtr arena_alloc(CompactArena *arena, size_t size) {
    const size_t align = 8;
    size_t aligned = (size + (align - 1)) & ~(align - 1);
    assert(arena->used + aligned <= arena->capacity);
    CompactPtr cp;
    cp.offset = (uint32_t)arena->used;
    memset(arena->base + arena->used, 0, aligned);
    arena->used += aligned;
    return cp;
}

void *compact_ptr_deref(const CompactArena *arena, CompactPtr cp) {
    assert(cp.offset < arena->capacity);
    return (void *)(arena->base + cp.offset);
}

size_t arena_remaining(const CompactArena *arena) {
    return arena->capacity - arena->used;
}
