#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct MinHeap {
    int *data;          /* Dynamic array holding the heap elements */
    size_t capacity;   /* Allocated slots in data */
    size_t count;      /* Number of elements currently stored */
};

/* Internal helper: ensure capacity is at least new_cap. */
static bool heap_resize(MinHeap *heap, size_t new_cap) {
    assert(heap != NULL);
    if (new_cap < heap->count) {
        return false; /* cannot shrink below current size */
    }
    int *new_data = (int *)realloc(heap->data, new_cap * sizeof(int));
    if (new_data == NULL && new_cap != 0) {
        return false;
    }
    heap->data = new_data;
    heap->capacity = new_cap;
    return true;
}

/* Internal helper: restore heap property moving element at idx up. */
static void heap_sift_up(MinHeap *heap, size_t idx) {
    assert(heap != NULL && idx < heap->count);
    while (idx > 0) {
        size_t parent = (idx - 1U) / 2U;
        if (heap->data[parent] <= heap->data[idx]) {
            break;
        }
        int tmp = heap->data[parent];
        heap->data[parent] = heap->data[idx];
        heap->data[idx] = tmp;
        idx = parent;
    }
}

/* Internal helper: restore heap property moving element at idx down. */
static void heap_sift_down(MinHeap *heap, size_t idx) {
    assert(heap != NULL && idx < heap->count);
    size_t left, right, smallest;
    while (true) {
        left = 2U * idx + 1U;
        right = left + 1U;
        smallest = idx;

        if (left < heap->count && heap->data[left] < heap->data[smallest]) {
            smallest = left;
        }
        if (right < heap->count && heap->data[right] < heap->data[smallest]) {
            smallest = right;
        }
        if (smallest == idx) {
            break;
        }
        int tmp = heap->data[idx];
        heap->data[idx] = heap->data[smallest];
        heap->data[smallest] = tmp;
        idx = smallest;
    }
}

/* Public API implementations */

MinHeap* heap_create(void) {
    MinHeap *heap = (MinHeap *)malloc(sizeof(MinHeap));
    if (heap == NULL) {
        return NULL;
    }
    heap->data = NULL;
    heap->capacity = 0;
    heap->count = 0;
    return heap;
}

void heap_destroy(MinHeap *heap) {
    if (heap != NULL) {
        free(heap->data);
        free(heap);
    }
}

bool heap_is_empty(const MinHeap *heap) {
    return heap == NULL || heap->count == 0;
}

size_t heap_size(const MinHeap *heap) {
    return heap ? heap->count : 0;
}

bool heap_push(MinHeap *heap, int value) {
    if (heap == NULL) {
        return false;
    }
    if (heap->count == heap->capacity) {
        size_t new_cap = heap->capacity ? heap->capacity * 2U : 4U;
        if (!heap_resize(heap, new_cap)) {
            return false;
        }
    }
    heap->data[heap->count] = value;
    heap_sift_up(heap, heap->count);
    heap->count++;
    return true;
}

bool heap_peek(const MinHeap *heap, int *out) {
    if (heap == NULL || heap->count == 0 || out == NULL) {
        return false;
    }
    *out = heap->data[0];
    return true;
}

bool heap_pop(MinHeap *heap, int *out) {
    if (heap == NULL || heap->count == 0 || out == NULL) {
        return false;
    }
    *out = heap->data[0];
    heap->count--;
    if (heap->count > 0) {
        heap->data[0] = heap->data[heap->count];
        heap_sift_down(heap, 0);
    }
    /* Optional shrink: keep at least 4 slots to avoid thrashing */
    if (heap->capacity > 8U && heap->count < heap->capacity / 4U) {
        size_t new_cap = heap->capacity / 2U;
        heap_resize(heap, new_cap);
    }
    return true;
}
