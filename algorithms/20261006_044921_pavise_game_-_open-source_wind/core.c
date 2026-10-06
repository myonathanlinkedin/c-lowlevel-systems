#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

typedef struct {
    uint32_t pid;
    bool suppressed;
} ProcessEntry;

typedef struct {
    ProcessEntry *entries;
    size_t count;
    size_t capacity;
} ProcessManager;

static const size_t INITIAL_CAPACITY = 16;

static void pm_resize(ProcessManager *pm, size_t new_capacity) {
    ProcessEntry *new_entries = realloc(pm->entries, new_capacity * sizeof(ProcessEntry));
    assert(new_entries != NULL);
    pm->entries = new_entries;
    pm->capacity = new_capacity;
}

void pm_init(ProcessManager *pm) {
    pm->entries = malloc(INITIAL_CAPACITY * sizeof(ProcessEntry));
    assert(pm->entries != NULL);
    pm->count = 0;
    pm->capacity = INITIAL_CAPACITY;
}

void pm_destroy(ProcessManager *pm) {
    free(pm->entries);
    pm->entries = NULL;
    pm->count = 0;
    pm->capacity = 0;
}

bool pm_add(ProcessManager *pm, uint32_t pid) {
    for (size_t i = 0; i < pm->count; ++i) {
        if (pm->entries[i].pid == pid) return false; // duplicate
    }
    if (pm->count == pm->capacity) pm_resize(pm, pm->capacity * 2);
    pm->entries[pm->count].pid = pid;
    pm->entries[pm->count].suppressed = false;
    ++pm->count;
    return true;
}

bool pm_suppress(ProcessManager *pm, uint32_t pid) {
    for (size_t i = 0; i < pm->count; ++i) {
        if (pm->entries[i].pid == pid) {
            pm->entries[i].suppressed = true;
            return true;
        }
    }
    return false;
}

bool pm_unsuppress(ProcessManager *pm, uint32_t pid) {
    for (size_t i = 0; i < pm->count; ++i) {
        if (pm->entries[i].pid == pid) {
            pm->entries[i].suppressed = false;
            return true;
        }
    }
    return false;
}

bool pm_is_suppressed(const ProcessManager *pm, uint32_t pid) {
    for (size_t i = 0; i < pm->count; ++i) {
        if (pm->entries[i].pid == pid) return pm->entries[i].suppressed;
    }
    return false;
}

size_t pm_list_suppressed(const ProcessManager *pm, uint32_t *out, size_t out_size) {
    size_t idx = 0;
    for (size_t i = 0; i < pm->count && idx < out_size; ++i) {
        if (pm->entries[i].suppressed) {
            out[idx++] = pm->entries[i].pid;
        }
    }
    return idx;
}
