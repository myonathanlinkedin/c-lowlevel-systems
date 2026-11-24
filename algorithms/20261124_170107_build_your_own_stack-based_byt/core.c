#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static inline int32_t vm_pop(vm_t *vm) {
    assert(vm->sp > 0 && "Stack underflow");
    return vm->stack[--vm->sp];
}

static inline void vm_push(vm_t *vm, int32_t value) {
    assert(vm->sp < vm->stack_capacity && "Stack overflow");
    vm->stack[vm->sp++] = value;
}

/* Helper to read a 32‑bit little‑endian immediate */
static inline int32_t read_i32(const uint8_t *buf) {
    return (int32_t)(
        ((uint32_t)buf[0]) |
        ((uint32_t)buf[1] << 8) |
        ((uint32_t)buf[2] << 16) |
        ((uint32_t)buf[3] << 24));
}

vm_t *vm_create(const uint8_t *code, size_t code_size, size_t stack_capacity) {
    vm_t *vm = (vm_t *)malloc(sizeof(vm_t));
    if (!vm) return NULL;
    vm->code = (uint8_t *)malloc(code_size);
    if (!vm->code) { free(vm); return NULL; }
    for (size_t i = 0; i < code_size; ++i) vm->code[i] = code[i];
    vm->code_size = code_size;
    vm->pc = 0;
    vm->stack_capacity = stack_capacity;
    vm->stack = (int32_t *)malloc(stack_capacity * sizeof(int32_t));
    if (!vm->stack) { free(vm->code); free(vm); return NULL; }
    vm->sp = 0;
    vm->halted = false;
    return vm;
}

void vm_destroy(vm_t *vm) {
    if (!vm) return;
    free(vm->stack);
    free(vm->code);
    free(vm);
}

vm_status_t vm_step(vm_t *vm) {
    if (vm->halted) return VM_HALT;
    if (vm->pc >= vm->code_size) return VM_ERROR;

    vm_opcode_t opcode = (vm_opcode_t)vm->code[vm->pc++];
    switch (opcode) {
        case OP_NOP:
            break;
        case OP_PUSH: {
            if (vm->pc + 4 > vm->code_size) return VM_ERROR;
            int32_t imm = read_i32(&vm->code[vm->pc]);
            vm->pc += 4;
            vm_push(vm, imm);
            break;
        }
        case OP_ADD: {
            int32_t b = vm_pop(vm);
            int32_t a = vm_pop(vm);
            vm_push(vm, a + b);
            break;
        }
        case OP_SUB: {
            int32_t b = vm_pop(vm);
            int32_t a = vm_pop(vm);
            vm_push(vm, a - b);
            break;
        }
        case OP_MUL: {
            int32_t b = vm_pop(vm);
            int32_t a = vm_pop(vm);
            vm_push(vm, a * b);
            break;
        }
        case OP_DIV: {
            int32_t b = vm_pop(vm);
            if (b == 0) return VM_ERROR;
            int32_t a = vm_pop(vm);
            vm_push(vm, a / b);
            break;
        }
        case OP_PRINT: {
            int32_t val = vm_pop(vm);
            printf("%d\n", val);
            break;
        }
        case OP_HALT:
            vm->halted = true;
            return VM_HALT;
        default:
            return VM_ERROR;
    }
    return VM_OK;
}

vm_status_t vm_run(vm_t *vm) {
    while (!vm->halted) {
        vm_status_t st = vm_step(vm);
        if (st == VM_ERROR) return VM_ERROR;
        if (st == VM_HALT) return VM_HALT;
    }
    return VM_OK;
}
