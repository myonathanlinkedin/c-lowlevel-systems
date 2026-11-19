#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    OP_NOP   = 0x00,
    OP_PUSH  = 0x01, // followed by int64_t immediate (little endian)
    OP_POP   = 0x02,
    OP_ADD   = 0x03,
    OP_SUB   = 0x04,
    OP_MUL   = 0x05,
    OP_DIV   = 0x06,
    OP_PRINT = 0x07,
    OP_HALT  = 0xFF
} OpCode;

typedef enum {
    VM_OK = 0,
    VM_ERR_STACK_UNDERFLOW,
    VM_ERR_STACK_OVERFLOW,
    VM_ERR_DIV_BY_ZERO,
    VM_ERR_INVALID_OPCODE,
    VM_ERR_OUT_OF_MEMORY
} VmStatus;

typedef struct {
    int64_t *data;
    size_t   capacity;
    size_t   top; // index of next free slot
} Stack;

typedef struct {
    const uint8_t *bytecode;
    size_t         length;
    size_t         ip; // instruction pointer
    Stack          stack;
    VmStatus       status;
} Vm;
