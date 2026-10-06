#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>

MemStatus stack_init(Stack* s, size_t cap) {
    if (!s) return MEM_ERR_NULL;
    s->data = (uint8_t*)malloc(cap);
    if (!s->data) return MEM_ERR_ALLOC;
    s->capacity = cap;
    s->size = 0;
    return MEM_OK;
}

void stack_destroy(Stack* s) {
    if (s && s->data) {
        free(s->data);
        s->data = NULL;
        s->capacity = 0;
        s->size = 0;
    }
}

MemStatus stack_push(Stack* s, uint8_t val) {
    if (!s || !s->data) return MEM_ERR_NULL;
    if (s->size >= s->capacity) return MEM_ERR_BOUNDS;
    s->data[s->size++] = val;
    return MEM_OK;
}

MemStatus stack_pop(Stack* s, uint8_t* out) {
    if (!s || !s->data || !out) return MEM_ERR_NULL;
    if (s->size == 0) return MEM_ERR_BOUNDS;
    *out = s->data[--s->size];
    return MEM_OK;
}

MemStatus stack_peek(const Stack* s, uint8_t* out) {
    if (!s || !s->data || !out) return MEM_ERR_NULL;
    if (s->size == 0) return MEM_ERR_BOUNDS;
    *out = s->data[s->size - 1];
    return MEM_OK;
}

bool stack_is_empty(const Stack* s) {
    return !s || s->size == 0;
}

size_t stack_size(const Stack* s) {
    return s ? s->size : 0;
}

MemStatus heap_init(Heap* h, size_t cap) {
    if (!h) return MEM_ERR_NULL;
    h->data = (uint8_t*)malloc(cap);
    if (!h->data) return MEM_ERR_ALLOC;
    h->capacity = cap;
    h->size = 0;
    return MEM_OK;
}

void heap_destroy(Heap* h) {
    if (h && h->data) {
        free(h->data);
        h->data = NULL;
        h->capacity = 0;
        h->size = 0;
    }
}

MemStatus heap_push(Heap* h, uint8_t val) {
    if (!h || !h->data) return MEM_ERR_NULL;
    if (h->size >= h->capacity) return MEM_ERR_BOUNDS;
    h->data[h->size++] = val;
    return MEM_OK;
}

MemStatus heap_pop(Heap* h, uint8_t* out) {
    if (!h || !h->data || !out) return MEM_ERR_NULL;
    if (h->size == 0) return MEM_ERR_BOUNDS;
    *out = h->data[--h->size];
    return MEM_OK;
}

MemStatus heap_peek(const Heap* h, uint8_t* out) {
    if (!h || !h->data || !out) return MEM_ERR_NULL;
    if (h->size == 0) return MEM_ERR_BOUNDS;
    *out = h->data[h->size - 1];
    return MEM_OK;
}

bool heap_is_empty(const Heap* h) {
    return !h || h->size == 0;
}

size_t heap_size(const Heap* h) {
    return h ? h->size : 0;
}
