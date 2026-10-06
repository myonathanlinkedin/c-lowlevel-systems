#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "types.h"
#include "core.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int main(void) {
    CompactArena arena;
    const size_t arena_size = 1024 * 1024; // 1 MiB
    arena_init(&arena, arena_size);

    const size_t obj_size = 24;
    const size_t count = 10000;
    CompactPtr *ptrs = (CompactPtr *)malloc(count * sizeof(CompactPtr));
    assert(ptrs != NULL);

    for (size_t i = 0; i < count; ++i) {
        ptrs[i] = arena_alloc(&arena, obj_size);
        uint8_t *p = (uint8_t *)compact_ptr_deref(&arena, ptrs[i]);
        for (size_t j = 0; j < obj_size; ++j) {
            p[j] = (uint8_t)(i ^ j);
        }
    }

    for (size_t i = 0; i < count; ++i) {
        uint8_t *p = (uint8_t *)compact_ptr_deref(&arena, ptrs[i]);
        for (size_t j = 0; j < obj_size; ++j) {
            assert(p[j] == (uint8_t)(i ^ j));
        }
    }

    for (size_t i = 0; i < count; ++i) {
        assert(ptrs[i].offset <= UINT32_MAX);
    }

    size_t remaining = arena_remaining(&arena);
    assert(remaining + arena.used <= arena.capacity);

    free(ptrs);
    arena_destroy(&arena);
    printf("All tests passed.\n");
    return 0;
}
