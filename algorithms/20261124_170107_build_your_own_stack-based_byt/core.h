#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#pragma once
#include "types.h"

typedef struct {
    uint8_t *code;          // bytecode buffer
    size_t code_size;       // length of code buffer
    size_t pc;              // program counter (byte offset)
    int32_t *stack;         // operand stack
    size_t sp;              // stack pointer (points to next free slot)
    size_t stack_capacity;  // maximum stack slots
    bool halted;            // true after HALT executed
} vm_t;

/* VM lifecycle */
vm_t *vm_create(const uint8_t *code, size_t code_size, size_t stack_capacity);
void vm_destroy(vm_t *vm);

/* Execution */
vm_status_t vm_step(vm_t *vm);
vm_status_t vm_run(vm_t *vm);

/* Stack access (used by tests) */
static inline int32_t vm_peek(const vm_t *vm, size_t depth) {
    // depth = 0 => top of stack
    return vm->stack[vm->sp - 1 - depth];
}
