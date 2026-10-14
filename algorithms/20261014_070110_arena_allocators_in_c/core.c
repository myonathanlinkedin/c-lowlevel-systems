#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <stdalign.h>

/* Helper: round up n to the next multiple of align (align must be power‑of‑2). */
static inline size_t align_up(size_t n, size_t align) {
    return (n + (align - 1)) & ~(align - 1);
}

void arena_init(Arena *arena, void *buffer, size_t size) {
    arena->buffer   = (uint8_t *)buffer;
    arena->capacity = size;
    arena->offset   = 0;
    arena->owned    = false;
}

bool arena_init_owned(Arena *arena, size_t size) {
    void *buf = malloc(size);
    if (!buf) {
        return false;
    }
    arena_init(arena, buf, size);
    arena->owned = true;
    return true;
}

void *arena_alloc(Arena *arena, size_t size, size_t align) {
    if (size == 0) {
        return NULL;
    }
    if (align == 0) {
        align = alignof(max_align_t);
    }
    /* Ensure alignment is a power of two; otherwise fallback to default. */
    if ((align & (align - 1)) != 0) {
        align = alignof(max_align_t);
    }

    size_t cur = arena->offset;
    size_t aligned = align_up(cur, align);
    size_t new_offset = aligned + size;

    if (new_offset > arena->capacity) {
        return NULL; /* Out of memory */
    }

    arena->offset = new_offset;
    return arena->buffer + aligned;
}

void arena_reset(Arena *arena) {
    arena->offset = 0;
    /* Optional: zero out memory for debugging; omitted for performance. */
}

void arena_destroy(Arena *arena) {
    if (arena->owned && arena->buffer) {
        free(arena->buffer);
    }
    /* Invalidate fields to catch misuse after destroy. */
    arena->buffer   = NULL;
    arena->capacity = 0;
    arena->offset   = 0;
    arena->owned    = false;
}
