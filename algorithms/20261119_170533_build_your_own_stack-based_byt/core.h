#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

Vm *vm_create(const uint8_t *bytecode, size_t length);
void vm_destroy(Vm *vm);
VmStatus vm_run(Vm *vm);
int64_t vm_peek(const Vm *vm, size_t depth); // depth 0 = top-1

#ifdef __cplusplus
}
#endif
