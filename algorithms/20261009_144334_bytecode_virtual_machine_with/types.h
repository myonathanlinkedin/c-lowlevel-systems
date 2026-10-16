#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef int32_t vm_word_t;          // Stack element type (signed 32‑bit)
typedef uint8_t vm_byte_t;          // Bytecode element type

// VM execution status
typedef enum {
    VM_OK = 0,
    VM_HALT,
    VM_ERR_DIV_ZERO,
    VM_ERR_STACK_UNDERFLOW,
    VM_ERR_STACK_OVERFLOW,
    VM_ERR_UNKNOWN_OPCODE,
    VM_ERR_PROGRAM_OVERFLOW
} vm_status_t;
