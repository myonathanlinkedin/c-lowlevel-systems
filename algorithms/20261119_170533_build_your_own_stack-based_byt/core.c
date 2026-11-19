#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "types.h"
#include "core.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define INITIAL_STACK_CAPACITY 64

static VmStatus stack_init(Stack *s) {
    s->capacity = INITIAL_STACK_CAPACITY;
    s->top = 0;
    s->data = (int64_t *)malloc(s->capacity * sizeof(int64_t));
    return s->data ? VM_OK : VM_ERR_OUT_OF_MEMORY;
}

static void stack_free(Stack *s) {
    free(s->data);
    s->data = NULL;
    s->capacity = s->top = 0;
}

static VmStatus stack_push(Stack *s, int64_t value) {
    if (s->top >= s->capacity) {
        size_t new_cap = s->capacity * 2;
        int64_t *tmp = (int64_t *)realloc(s->data, new_cap * sizeof(int64_t));
        if (!tmp) return VM_ERR_OUT_OF_MEMORY;
        s->data = tmp;
        s->capacity = new_cap;
    }
    s->data[s->top++] = value;
    return VM_OK;
}

static VmStatus stack_pop(Stack *s, int64_t *out) {
    if (s->top == 0) return VM_ERR_STACK_UNDERFLOW;
    *out = s->data[--s->top];
    return VM_OK;
}

static int64_t stack_peek(const Stack *s, size_t depth) {
    if (depth >= s->top) return 0;
    return s->data[s->top - 1 - depth];
}

/* Helper to read a signed 64‑bit immediate from bytecode (little endian) */
static int64_t read_i64(const uint8_t *buf) {
    int64_t v = 0;
    for (unsigned i = 0; i < 8; ++i) {
        v |= ((int64_t)buf[i]) << (i * 8U);
    }
    return v;
}

/* VM creation / destruction */
Vm *vm_create(const uint8_t *bytecode, size_t length) {
    Vm *vm = (Vm *)malloc(sizeof(Vm));
    if (!vm) return NULL;
    vm->bytecode = bytecode;
    vm->length   = length;
    vm->ip       = 0;
    vm->status   = VM_OK;
    if (stack_init(&vm->stack) != VM_OK) {
        free(vm);
        return NULL;
    }
    return vm;
}

void vm_destroy(Vm *vm) {
    if (!vm) return;
    stack_free(&vm->stack);
    free(vm);
}

/* Core execution loop */
VmStatus vm_run(Vm *vm) {
    while (vm->ip < vm->length && vm->status == VM_OK) {
        OpCode op = (OpCode)vm->bytecode[vm->ip++];
        switch (op) {
            case OP_NOP:
                break;
            case OP_PUSH: {
                if (vm->ip + 8 > vm->length) { vm->status = VM_ERR_INVALID_OPCODE; break; }
                int64_t imm = read_i64(&vm->bytecode[vm->ip]);
                vm->ip += 8;
                vm->status = stack_push(&vm->stack, imm);
                break;
            }
            case OP_POP: {
                int64_t discard;
                vm->status = stack_pop(&vm->stack, &discard);
                break;
            }
            case OP_ADD:
            case OP_SUB:
            case OP_MUL:
            case OP_DIV: {
                int64_t b, a;
                if ((vm->status = stack_pop(&vm->stack, &b)) != VM_OK) break;
                if ((vm->status = stack_pop(&vm->stack, &a)) != VM_OK) break;
                int64_t res = 0;
                if (op == OP_ADD) res = a + b;
                else if (op == OP_SUB) res = a - b;
                else if (op == OP_MUL) res = a * b;
                else { // DIV
                    if (b == 0) { vm->status = VM_ERR_DIV_BY_ZERO; break; }
                    res = a / b;
                }
                vm->status = stack_push(&vm->stack, res);
                break;
            }
            case OP_PRINT: {
                int64_t val;
                if ((vm->status = stack_pop(&vm->stack, &val)) != VM_OK) break;
                printf("%lld\n", (long long)val);
                break;
            }
            case OP_HALT:
                return VM_OK;
            default:
                vm->status = VM_ERR_INVALID_OPCODE;
                break;
        }
    }
    return vm->status;
}

/* Peek without mutating the VM (used by tests) */
int64_t vm_peek(const Vm *vm, size_t depth) {
    return stack_peek(&vm->stack, depth);
}
