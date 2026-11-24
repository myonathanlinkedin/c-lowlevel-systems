#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "core.h"
#include <assert.h>
#include <stdio.h>

/* Helper to assemble bytecode for tests */
static void emit_push(uint8_t **buf, int32_t value) {
    **buf = OP_PUSH; (*buf)++;
    // little‑endian encoding
    (*buf)[0] = (uint8_t)(value & 0xFF);
    (*buf)[1] = (uint8_t)((value >> 8) & 0xFF);
    (*buf)[2] = (uint8_t)((value >> 16) & 0xFF);
    (*buf)[3] = (uint8_t)((value >> 24) & 0xFF);
    *buf += 4;
}

static void emit(uint8_t **buf, vm_opcode_t op) {
    **buf = (uint8_t)op;
    (*buf)++;
}

/* Unit test: 2 + 3 = 5 */
static void test_addition(void) {
    uint8_t program[32];
    uint8_t *p = program;
    emit_push(&p, 2);
    emit_push(&p, 3);
    emit(&p, OP_ADD);
    emit(&p, OP_HALT);
    size_t prog_len = (size_t)(p - program);

    vm_t *vm = vm_create(program, prog_len, 16);
    assert(vm != NULL);
    vm_status_t st = vm_run(vm);
    assert(st == VM_HALT);
    assert(vm->sp == 1);
    assert(vm_peek(vm, 0) == 5);
    vm_destroy(vm);
}

/* Unit test: (10 - 4) * 3 = 18 */
static void test_complex(void) {
    uint8_t program[64];
    uint8_t *p = program;
    emit_push(&p, 10);
    emit_push(&p, 4);
    emit(&p, OP_SUB);      // 6
    emit_push(&p, 3);
    emit(&p, OP_MUL);      // 18
    emit(&p, OP_HALT);
    size_t prog_len = (size_t)(p - program);

    vm_t *vm = vm_create(program, prog_len, 16);
    assert(vm != NULL);
    vm_status_t st = vm_run(vm);
    assert(st == VM_HALT);
    assert(vm_peek(vm, 0) == 18);
    vm_destroy(vm);
}

/* Unit test: division by zero yields error */
static void test_div_by_zero(void) {
    uint8_t program[32];
    uint8_t *p = program;
    emit_push(&p, 42);
    emit_push(&p, 0);
    emit(&p, OP_DIV);
    emit(&p, OP_HALT);
    size_t prog_len = (size_t)(p - program);

    vm_t *vm = vm_create(program, prog_len, 16);
    assert(vm != NULL);
    vm_status_t st = vm_run(vm);
    assert(st == VM_ERROR);
    vm_destroy(vm);
}

/* Unit test: PRINT opcode writes to stdout (visual check) */
static void test_print(void) {
    uint8_t program[32];
    uint8_t *p = program;
    emit_push(&p, 12345);
    emit(&p, OP_PRINT);
    emit(&p, OP_HALT);
    size_t prog_len = (size_t)(p - program);

    vm_t *vm = vm_create(program, prog_len, 8);
    assert(vm != NULL);
    vm_status_t st = vm_run(vm);
    assert(st == VM_HALT);
    vm_destroy(vm);
    /* Expected output: 12345 */
}

int main(void) {
    test_addition();
    test_complex();
    test_div_by_zero();
    test_print();
    printf("All VM tests passed.\n");
    return 0;
}
