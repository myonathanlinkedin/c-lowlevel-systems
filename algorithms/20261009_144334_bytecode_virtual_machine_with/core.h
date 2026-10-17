#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#pragma once
#include "types.h"

#define VM_STACK_CAPACITY 256
#define VM_PROGRAM_CAPACITY 1024

// Opcode enumeration (must fit in a byte)
typedef enum {
    OP_HALT  = 0x00,
    OP_PUSH  = 0x01,   // operand: 4‑byte signed integer (big‑endian)
    OP_ADD   = 0x02,
    OP_SUB   = 0x03,
    OP_MUL   = 0x04,
    OP_DIV   = 0x05,
    OP_MOD   = 0x06,
    OP_NEG   = 0x07,
    OP_DUP   = 0x08,
    OP_SWAP  = 0x09,
    OP_POP   = 0x0A,
    OP_PRINT = 0x0B
} vm_opcode_t;

// Core VM structure
typedef struct {
    vm_word_t stack[VM_STACK_CAPACITY];
    size_t    sp;               // stack pointer (points to next free slot)
    vm_byte_t program[VM_PROGRAM_CAPACITY];
    size_t    pc;               // program counter (index into program[])
    size_t    program_len;
} vm_t;

// Public API
void vm_init(vm_t *vm);
vm_status_t vm_load_program(vm_t *vm, const vm_byte_t *code, size_t len);
vm_status_t vm_step(vm_t *vm);
vm_status_t vm_run(vm_t *vm);
