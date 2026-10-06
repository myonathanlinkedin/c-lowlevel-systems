#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#pragma once

#include "types.h"

MemStatus stack_init(Stack* s, size_t cap);
void stack_destroy(Stack* s);
MemStatus stack_push(Stack* s, uint8_t val);
MemStatus stack_pop(Stack* s, uint8_t* out);
MemStatus stack_peek(const Stack* s, uint8_t* out);
bool stack_is_empty(const Stack* s);
size_t stack_size(const Stack* s);

MemStatus heap_init(Heap* h, size_t cap);
void heap_destroy(Heap* h);
MemStatus heap_push(Heap* h, uint8_t val);
MemStatus heap_pop(Heap* h, uint8_t* out);
MemStatus heap_peek(const Heap* h, uint8_t* out);
bool heap_is_empty(const Heap* h);
size_t heap_size(const Heap* h);
