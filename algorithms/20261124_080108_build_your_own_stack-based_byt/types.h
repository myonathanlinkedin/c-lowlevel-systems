#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    VM_OK = 0,
    VM_HALT,
    VM_ERR_DIV0,
    VM_ERR_OVERFLOW,
    VM_ERR_UNDERFLOW,
    VM_ERR_UNKNOWN_OPCODE
} VMStatus;

typedef enum {
    OP_PUSH_IMM = 0x01, // followed by 4-byte little-endian int32
    OP_POP      = 0x02,
    OP_ADD      = 0x03,
    OP_SUB      = 0x04,
    OP_MUL      = 0x05,
    OP_DIV      = 0x06,
    OP_DUP      = 0x07,
    OP_SWAP     = 0x08,
    OP_HALT     = 0xFF
} OpCode;

#define VM_STACK_MAX 256

typedef struct {
    const uint8_t *code;      // bytecode buffer
    size_t code_len;          // length of bytecode
    size_t pc;                // program counter
    int32_t stack[VM_STACK_MAX];
    size_t sp;                // stack pointer (points to next free slot)
    VMStatus status;
} VM;
