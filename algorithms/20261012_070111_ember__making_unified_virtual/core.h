#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void vm_init(VirtualMemoryManager *vm);
size_t vm_alloc(VirtualMemoryManager *vm, size_t num_pages);
void vm_write(VirtualMemoryManager *vm, size_t vaddr, const void *src, size_t len);
void vm_read(VirtualMemoryManager *vm, size_t vaddr, void *dst, size_t len);
void vm_cleanup(VirtualMemoryManager *vm);

#ifdef __cplusplus
}
#endif
