#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <assert.h>

static inline bool vm_stack_push(VM *vm, int32_t value) {
    if (vm->sp >= VM_STACK_MAX) {
        vm->status = VM_ERR_OVERFLOW;
        return false;
    }
    vm->stack[vm->sp++] = value;
    return true;
}

static inline bool vm_stack_pop(VM *vm, int32_t *out) {
    if (vm->sp == 0) {
        vm->status = VM_ERR_UNDERFLOW;
        return false;
    }
    *out = vm->stack[--vm->sp];
    return true;
}

static inline bool vm_stack_peek(const VM *vm, size_t depth, int32_t *out) {
    if (depth >= vm->sp) {
        return false;
    }
    *out = vm->stack[vm->sp - 1 - depth];
    return true;
}

void vm_init(VM *vm, const uint8_t *code, size_t code_len) {
    assert(vm != NULL);
    vm->code = code;
    vm->code_len = code_len;
    vm->pc = 0;
    vm->sp = 0;
    vm->status = VM_OK;
}

static inline int32_t read_i32_le(const uint8_t *buf) {
    return (int32_t)(
        ((uint32_t)buf[0]) |
        ((uint32_t)buf[1] << 8) |
        ((uint32_t)buf[2] << 16) |
        ((uint32_t)buf[3] << 24));
}

VMStatus vm_step(VM *vm) {
    if (vm->status != VM_OK) {
        return vm->status;
    }
    if (vm->pc >= vm->code_len) {
        vm->status = VM_ERR_UNKNOWN_OPCODE;
        return vm->status;
    }

    OpCode op = (OpCode)vm->code[vm->pc++];
    int32_t a, b, result;

    switch (op) {
        case OP_PUSH_IMM:
            if (vm->pc + 4 > vm->code_len) {
                vm->status = VM_ERR_UNKNOWN_OPCODE;
                return vm->status;
            }
            result = read_i32_le(&vm->code[vm->pc]);
            vm->pc += 4;
            if (!vm_stack_push(vm, result)) return vm->status;
            break;

        case OP_POP:
            if (!vm_stack_pop(vm, &result)) return vm->status;
            break;

        case OP_DUP:
            if (!vm_stack_peek(vm, 0, &result)) {
                vm->status = VM_ERR_UNDERFLOW;
                return vm->status;
            }
            if (!vm_stack_push(vm, result)) return vm->status;
            break;

        case OP_SWAP:
            if (vm->sp < 2) {
                vm->status = VM_ERR_UNDERFLOW;
                return vm->status;
            }
            a = vm->stack[vm->sp - 1];
            b = vm->stack[vm->sp - 2];
            vm->stack[vm->sp - 1] = b;
            vm->stack[vm->sp - 2] = a;
            break;

        case OP_ADD:
            if (!vm_stack_pop(vm, &b)) return vm->status;
            if (!vm_stack_pop(vm, &a)) return vm->status;
            result = a + b;
            if (!vm_stack_push(vm, result)) return vm->status;
            break;

        case OP_SUB:
            if (!vm_stack_pop(vm, &b)) return vm->status;
            if (!vm_stack_pop(vm, &a)) return vm->status;
            result = a - b;
            if (!vm_stack_push(vm, result)) return vm->status;
            break;

        case OP_MUL:
            if (!vm_stack_pop(vm, &b)) return vm->status;
            if (!vm_stack_pop(vm, &a)) return vm->status;
            result = a * b;
            if (!vm_stack_push(vm, result)) return vm->status;
            break;

        case OP_DIV:
            if (!vm_stack_pop(vm, &b)) return vm->status;
            if (!vm_stack_pop(vm, &a)) return vm->status;
            if (b == 0) {
                vm->status = VM_ERR_DIV0;
                return vm->status;
            }
            result = a / b;
            if (!vm_stack_push(vm, result)) return vm->status;
            break;

        case OP_HALT:
            vm->status = VM_HALT;
            break;

        default:
            vm->status = VM_ERR_UNKNOWN_OPCODE;
            break;
    }
    return vm->status;
}

VMStatus vm_run(VM *vm) {
    while (vm->status == VM_OK) {
        vm_step(vm);
    }
    return vm->status;
}
