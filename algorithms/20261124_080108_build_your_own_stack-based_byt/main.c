#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "core.h"
#include <stdio.h>
#include <assert.h>

static void test_addition(void) {
    // Program: push 10, push 20, add, halt
    uint8_t prog[] = {
        OP_PUSH_IMM, 10, 0, 0, 0,
        OP_PUSH_IMM, 20, 0, 0, 0,
        OP_ADD,
        OP_HALT
    };
    VM vm;
    vm_init(&vm, prog, sizeof(prog));
    VMStatus st = vm_run(&vm);
    assert(st == VM_HALT);
    assert(vm.sp == 1);
    assert(vm.stack[0] == 30);
}

static void test_complex_expression(void) {
    // Compute (5 * 6) - 2 = 28
    uint8_t prog[] = {
        OP_PUSH_IMM, 5, 0, 0, 0,
        OP_PUSH_IMM, 6, 0, 0, 0,
        OP_MUL,
        OP_PUSH_IMM, 2, 0, 0, 0,
        OP_SUB,
        OP_HALT
    };
    VM vm;
    vm_init(&vm, prog, sizeof(prog));
    VMStatus st = vm_run(&vm);
    assert(st == VM_HALT);
    assert(vm.sp == 1);
    assert(vm.stack[0] == 28);
}

static void test_division_by_zero(void) {
    uint8_t prog[] = {
        OP_PUSH_IMM, 10, 0, 0, 0,
        OP_PUSH_IMM, 0, 0, 0, 0,
        OP_DIV,
        OP_HALT
    };
    VM vm;
    vm_init(&vm, prog, sizeof(prog));
    VMStatus st = vm_run(&vm);
    assert(st == VM_ERR_DIV0);
}

static void test_stack_manipulation(void) {
    // push 42, dup, swap, pop, halt -> final stack: [42]
    uint8_t prog[] = {
        OP_PUSH_IMM, 42, 0, 0, 0,
        OP_DUP,
        OP_SWAP,
        OP_POP,
        OP_HALT
    };
    VM vm;
    vm_init(&vm, prog, sizeof(prog));
    VMStatus st = vm_run(&vm);
    assert(st == VM_HALT);
    assert(vm.sp == 1);
    assert(vm.stack[0] == 42);
}

int main(void) {
    test_addition();
    test_complex_expression();
    test_division_by_zero();
    test_stack_manipulation();
    printf("All VM tests passed.\n");
    return 0;
}
