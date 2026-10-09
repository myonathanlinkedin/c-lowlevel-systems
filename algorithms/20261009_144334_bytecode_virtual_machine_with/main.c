#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "core.h"
#include <assert.h>
#include <stdio.h>

// Helper to encode a signed 32‑bit integer in big‑endian order
static void encode_push(vm_byte_t *buf, vm_word_t val) {
    buf[0] = (vm_byte_t)((val >> 24) & 0xFF);
    buf[1] = (vm_byte_t)((val >> 16) & 0xFF);
    buf[2] = (vm_byte_t)((val >> 8)  & 0xFF);
    buf[3] = (vm_byte_t)(val & 0xFF);
}

// Test 1: simple arithmetic 2 3 + 4 * => (2+3)*4 = 20
static void test_arithmetic(void) {
    vm_t vm;
    vm_init(&vm);
    vm_byte_t prog[32];
    size_t ip = 0;

    prog[ip++] = OP_PUSH; encode_push(&prog[ip], 2); ip += 4;
    prog[ip++] = OP_PUSH; encode_push(&prog[ip], 3); ip += 4;
    prog[ip++] = OP_ADD;
    prog[ip++] = OP_PUSH; encode_push(&prog[ip], 4); ip += 4;
    prog[ip++] = OP_MUL;
    prog[ip++] = OP_PRINT;
    prog[ip++] = OP_HALT;

    assert(vm_load_program(&vm, prog, ip) == VM_OK);
    assert(vm_run(&vm) == VM_HALT);
    // After execution stack should contain 20
    assert(vm.sp == 1);
    assert(vm_peek(&vm) == 20);
}

// Test 2: division by zero handling
static void test_div_zero(void) {
    vm_t vm;
    vm_init(&vm);
    vm_byte_t prog[] = {
        OP_PUSH, 0,0,0,10,   // 10
        OP_PUSH, 0,0,0,0,    // 0
        OP_DIV,
        OP_HALT
    };
    assert(vm_load_program(&vm, prog, sizeof(prog)) == VM_OK);
    assert(vm_run(&vm) == VM_ERR_DIV_ZERO);
}

// Test 3: stack underflow detection
static void test_underflow(void) {
    vm_t vm;
    vm_init(&vm);
    vm_byte_t prog[] = { OP_ADD, OP_HALT };
    assert(vm_load_program(&vm, prog, sizeof(prog)) == VM_OK);
    assert(vm_run(&vm) == VM_ERR_STACK_UNDERFLOW);
}

// Test 4: unknown opcode handling
static void test_unknown_opcode(void) {
    vm_t vm;
    vm_init(&vm);
    vm_byte_t prog[] = { 0xFF, OP_HALT };
    assert(vm_load_program(&vm, prog, sizeof(prog)) == VM_OK);
    assert(vm_run(&vm) == VM_ERR_UNKNOWN_OPCODE);
}

// Test 5: DUP and SWAP correctness
static void test_dup_swap(void) {
    vm_t vm;
    vm_init(&vm);
    vm_byte_t prog[16];
    size_t ip = 0;
    prog[ip++] = OP_PUSH; encode_push(&prog[ip], 42); ip += 4; // stack: 42
    prog[ip++] = OP_DUP;                                        // 42 42
    prog[ip++] = OP_PUSH; encode_push(&prog[ip], 7); ip += 4;  // 42 42 7
    prog[ip++] = OP_SWAP;                                      // 42 7 42
    prog[ip++] = OP_POP;                                       // 42 7
    prog[ip++] = OP_PRINT;                                     // prints 7
    prog[ip++] = OP_HALT;
    assert(vm_load_program(&vm, prog, ip) == VM_OK);
    assert(vm_run(&vm) == VM_HALT);
    assert(vm.sp == 2);
    assert(vm.stack[0] == 42);
    assert(vm.stack[1] == 7);
}

// Entry point runs all unit tests
int main(void) {
    test_arithmetic();
    test_div_zero();
    test_underflow();
    test_unknown_opcode();
    test_dup_swap();
    printf("All VM tests passed.\n");
    return 0;
}
