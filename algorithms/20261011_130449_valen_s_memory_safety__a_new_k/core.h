#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include "types.h"

/// Allocate a managed memory block of `size` bytes.
/// Returns NULL on allocation failure.
ManagedObject* mo_create(size_t size);

/// Release the managed block and all associated bookkeeping.
/// The object must not be currently borrowed.
void mo_destroy(ManagedObject* mo);

/// Acquire an immutable (shared) borrow.
/// Returns a pointer to the data on success, NULL if a mutable borrow is active.
const void* mo_borrow_shared(ManagedObject* mo);

/// Release a previously acquired immutable borrow.
void mo_return_shared(ManagedObject* mo);

/// Acquire an exclusive mutable borrow.
/// Returns a pointer to the data on success, NULL if any borrow is active.
void* mo_borrow_mut(ManagedObject* mo);

/// Release a previously acquired mutable borrow.
void mo_return_mut(ManagedObject* mo);
