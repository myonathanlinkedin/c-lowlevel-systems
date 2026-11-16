#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static Allocation *alloc_list = NULL;
static size_t total_allocated = 0;
static size_t current_allocated = 0;
static size_t peak_allocated = 0;
static size_t allocation_count = 0;

/* Internal helper: find allocation node */
static Allocation *find_allocation(void *ptr) {
    Allocation *it = alloc_list;
    while (it) {
        if (it->ptr == ptr) {
            return it;
        }
        it = it->next;
    }
    return NULL;
}

/* Internal helper: remove node from list */
static void remove_allocation(Allocation *node) {
    Allocation **prev = &alloc_list;
    while (*prev && *prev != node) {
        prev = &((*prev)->next);
    }
    if (*prev) {
        *prev = node->next;
    }
}

/* Public API */
void *tracked_malloc(size_t size, const char *file, int line) {
    if (size == 0) {
        return NULL;
    }
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Allocation failed at %s:%d (size=%zu)\n", file, line, size);
        return NULL;
    }
    Allocation *node = (Allocation *)malloc(sizeof(Allocation));
    assert(node != NULL);
    node->ptr = ptr;
    node->size = size;
    node->file = file;
    node->line = line;
    node->next = alloc_list;
    alloc_list = node;

    total_allocated += size;
    current_allocated += size;
    allocation_count++;
    if (current_allocated > peak_allocated) {
        peak_allocated = current_allocated;
    }
    return ptr;
}

void tracked_free(void *ptr, const char *file, int line) {
    if (!ptr) {
        return;
    }
    Allocation *node = find_allocation(ptr);
    if (!node) {
        fprintf(stderr, "Double free or invalid free at %s:%d (ptr=%p)\n", file, line, ptr);
        assert(false);
        return;
    }
    current_allocated -= node->size;
    remove_allocation(node);
    free(node);
    free(ptr);
}

size_t mem_total_allocated(void) {
    return total_allocated;
}

size_t mem_current_allocated(void) {
    return current_allocated;
}

size_t mem_peak_allocated(void) {
    return peak_allocated;
}

size_t mem_allocation_count(void) {
    return allocation_count;
}

void mem_report(void) {
    printf("Memory Report:\n");
    printf("  Total allocated   : %zu bytes\n", total_allocated);
    printf("  Current allocated : %zu bytes\n", current_allocated);
    printf("  Peak allocated     : %zu bytes\n", peak_allocated);
    printf("  Allocation count   : %zu\n", allocation_count);
    if (alloc_list) {
        printf("  Leaked allocations:\n");
        for (Allocation *it = alloc_list; it; it = it->next) {
            printf("    %p : %zu bytes (allocated at %s:%d)\n",
                   it->ptr, it->size, it->file, it->line);
        }
    } else {
        printf("  No leaks detected.\n");
    }
}

void mem_reset(void) {
    Allocation *it = alloc_list;
    while (it) {
        Allocation *next = it->next;
        free(it->ptr);
        free(it);
        it = next;
    }
    alloc_list = NULL;
    total_allocated = 0;
    current_allocated = 0;
    peak_allocated = 0;
    allocation_count = 0;
}
