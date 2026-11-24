#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    VM_OK = 0,
    VM_HALT,
    VM_ERROR
} vm_status_t;

typedef enum {
    OP_NOP   = 0x00,
    OP_PUSH  = 0x01,   // push immediate int32_t (4 bytes)
    OP_ADD   = 0x02,
    OP_SUB   = 0x03,
    OP_MUL   = 0x04,
    OP_DIV   = 0x05,
    OP_PRINT = 0x06,
    OP_HALT  = 0xFF
} vm_opcode_t;
