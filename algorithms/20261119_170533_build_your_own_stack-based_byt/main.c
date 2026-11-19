#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "core.h"
#include <assert.h>
#include <stdio.h>

/* Helper to encode a 64‑bit immediate in little endian */
static void emit_push(uint8_t *buf, size_t *pos, int64_t value) {
    buf[(*pos)++] = OP_PUSH;
    for (unsigned i = 0; i < 8; ++i) {
        buf[(*pos)++] = (uint8_t)((value >> (i * 8U)) & 0xFF);
    }
}

/* Build a simple program:
   push 10
   push 20
   add
   push 5
   mul
   print
   halt
*/
static void test_arithmetic(void) {
    uint8_t prog[64];
    size_t p = 0;
    emit_push(prog, &p, 10);
    emit_push(prog, &p, 20);
    prog[p++] = OP_ADD;
    emit_push(prog, &p, 5);
    prog[p++] = OP_MUL;
    prog[p++] = OP_PRINT;
    prog[p++] = OP_HALT;

    Vm *vm = vm_create(prog, p);
    assert(vm != NULL);
    VmStatus st = vm_run(vm);
    assert(st == VM_OK);
    // After execution stack should be empty (print popped the result)
    assert(vm_peek(vm, 0) == 0);
    vm_destroy(vm);
}

/* Test division by zero handling */
static void test_div_by_zero(void) {
    uint8_t prog[32];
    size_t p = 0;
    emit_push(prog, &p, 42);
    emit_push(prog, &p, 0);
    prog[p++] = OP_DIV;
    prog[p++] = OP_HALT;

    Vm *vm = vm_create(prog, p);
    assert(vm != NULL);
    VmStatus st = vm_run(vm);
    assert(st == VM_ERR_DIV_BY_ZERO);
    vm_destroy(vm);
}

/* Test stack underflow */
static void test_underflow(void) {
    uint8_t prog[8];
    size_t p = 0;
    prog[p++] = OP_POP; // nothing on stack
    prog[p++] = OP_HALT;

    Vm *vm = vm_create(prog, p);
    assert(vm != NULL);
    VmStatus st = vm_run(vm);
    assert(st == VM_ERR_STACK_UNDERFLOW);
    vm_destroy(vm);
}

/* Test that the VM correctly grows its stack */
static void test_stack_growth(void) {
    uint8_t prog[1024];
    size_t p = 0;
    const int N = 200; // > INITIAL_STACK_CAPACITY
    for (int i = 0; i < N; ++i) {
        emit_push(prog, &p, i);
    }
    prog[p++] = OP_HALT;

    Vm *vm = vm_create(prog, p);
    assert(vm != NULL);
    VmStatus st = vm_run(vm);
    assert(st == VM_OK);
    // Top of stack should be N-1
    assert(vm_peek(vm, 0) == N - 1);
    vm_destroy(vm);
}

/* Entry point running all unit tests */
int main(void) {
    test_arithmetic();
    test_div_by_zero();
    test_underflow();
    test_stack_growth();
    printf("All tests passed.\n");
    return 0;
}
