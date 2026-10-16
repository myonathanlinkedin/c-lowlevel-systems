#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdio.h>
#include <string.h>

static inline bool vm_stack_is_full(const vm_t *vm) {
    return vm->sp >= VM_STACK_CAPACITY;
}
static inline bool vm_stack_is_empty(const vm_t *vm) {
    return vm->sp == 0;
}
static inline void vm_push(vm_t *vm, vm_word_t val) {
    vm->stack[vm->sp++] = val;
}
static inline vm_word_t vm_pop(vm_t *vm) {
    return vm->stack[--vm->sp];
}
static inline vm_word_t vm_peek(const vm_t *vm) {
    return vm->stack[vm->sp - 1];
}

// Initialise a fresh VM instance
void vm_init(vm_t *vm) {
    vm->sp = 0;
    vm->pc = 0;
    vm->program_len = 0;
    memset(vm->stack, 0, sizeof(vm->stack));
    memset(vm->program, 0, sizeof(vm->program));
}

// Load bytecode into the VM; returns error if program exceeds capacity
vm_status_t vm_load_program(vm_t *vm, const vm_byte_t *code, size_t len) {
    if (len > VM_PROGRAM_CAPACITY) {
        return VM_ERR_PROGRAM_OVERFLOW;
    }
    memcpy(vm->program, code, len);
    vm->program_len = len;
    vm->pc = 0;
    return VM_OK;
}

// Execute a single instruction; updates VM state
vm_status_t vm_step(vm_t *vm) {
    if (vm->pc >= vm->program_len) {
        return VM_ERR_UNKNOWN_OPCODE; // no more bytes – treat as error
    }

    vm_opcode_t opcode = (vm_opcode_t)vm->program[vm->pc++];
    switch (opcode) {
        case OP_HALT:
            return VM_HALT;

        case OP_PUSH: {
            if (vm->pc + 4 > vm->program_len) return VM_ERR_UNKNOWN_OPCODE;
            // Decode big‑endian 32‑bit signed integer
            vm_word_t val = (vm_word_t)(
                ((uint32_t)vm->program[vm->pc]   << 24) |
                ((uint32_t)vm->program[vm->pc+1] << 16) |
                ((uint32_t)vm->program[vm->pc+2] << 8)  |
                ((uint32_t)vm->program[vm->pc+3])
            );
            vm->pc += 4;
            if (vm_stack_is_full(vm)) return VM_ERR_STACK_OVERFLOW;
            vm_push(vm, val);
            break;
        }

        case OP_ADD: {
            if (vm->sp < 2) return VM_ERR_STACK_UNDERFLOW;
            vm_word_t b = vm_pop(vm);
            vm_word_t a = vm_pop(vm);
            vm_push(vm, a + b);
            break;
        }

        case OP_SUB: {
            if (vm->sp < 2) return VM_ERR_STACK_UNDERFLOW;
            vm_word_t b = vm_pop(vm);
            vm_word_t a = vm_pop(vm);
            vm_push(vm, a - b);
            break;
        }

        case OP_MUL: {
            if (vm->sp < 2) return VM_ERR_STACK_UNDERFLOW;
            vm_word_t b = vm_pop(vm);
            vm_word_t a = vm_pop(vm);
            vm_push(vm, a * b);
            break;
        }

        case OP_DIV: {
            if (vm->sp < 2) return VM_ERR_STACK_UNDERFLOW;
            vm_word_t b = vm_pop(vm);
            if (b == 0) return VM_ERR_DIV_ZERO;
            vm_word_t a = vm_pop(vm);
            vm_push(vm, a / b);
            break;
        }

        case OP_MOD: {
            if (vm->sp < 2) return VM_ERR_STACK_UNDERFLOW;
            vm_word_t b = vm_pop(vm);
            if (b == 0) return VM_ERR_DIV_ZERO;
            vm_word_t a = vm_pop(vm);
            vm_push(vm, a % b);
            break;
        }

        case OP_NEG: {
            if (vm_stack_is_empty(vm)) return VM_ERR_STACK_UNDERFLOW;
            vm_word_t a = vm_pop(vm);
            vm_push(vm, -a);
            break;
        }

        case OP_DUP: {
            if (vm_stack_is_empty(vm)) return VM_ERR_STACK_UNDERFLOW;
            if (vm_stack_is_full(vm)) return VM_ERR_STACK_OVERFLOW;
            vm_word_t a = vm_peek(vm);
            vm_push(vm, a);
            break;
        }

        case OP_SWAP: {
            if (vm->sp < 2) return VM_ERR_STACK_UNDERFLOW;
            vm_word_t a = vm->stack[vm->sp - 1];
            vm_word_t b = vm->stack[vm->sp - 2];
            vm->stack[vm->sp - 1] = b;
            vm->stack[vm->sp - 2] = a;
            break;
        }

        case OP_POP: {
            if (vm_stack_is_empty(vm)) return VM_ERR_STACK_UNDERFLOW;
            (void)vm_pop(vm);
            break;
        }

        case OP_PRINT: {
            if (vm_stack_is_empty(vm)) return VM_ERR_STACK_UNDERFLOW;
            vm_word_t a = vm_peek(vm);
            printf("%d\n", a);
            break;
        }

        default:
            return VM_ERR_UNKNOWN_OPCODE;
    }
    return VM_OK;
}

// Run until HALT or error
vm_status_t vm_run(vm_t *vm) {
    vm_status_t status = VM_OK;
    while ((status = vm_step(vm)) == VM_OK) {
        /* loop */
    }
    return status;
}
