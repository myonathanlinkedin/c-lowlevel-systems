#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdatomic.h>

// Generic opaque pointer type for stored items
typedef void* aether_item_t;
