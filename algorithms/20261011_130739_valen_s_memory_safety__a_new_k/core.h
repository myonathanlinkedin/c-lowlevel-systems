#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include "types.h"

BorrowedObject* mo_create(size_t size);
void mo_destroy(BorrowedObject* obj);
void* mo_borrow_immutable(BorrowedObject* obj);
void mo_release_immutable(BorrowedObject* obj);
void* mo_borrow_mutable(BorrowedObject* obj);
void mo_release_mutable(BorrowedObject* obj);
