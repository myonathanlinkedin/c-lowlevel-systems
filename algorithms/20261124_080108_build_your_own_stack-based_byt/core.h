#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void vm_init(VM *vm, const uint8_t *code, size_t code_len);
VMStatus vm_step(VM *vm);
VMStatus vm_run(VM *vm);

#ifdef __cplusplus
}
#endif
