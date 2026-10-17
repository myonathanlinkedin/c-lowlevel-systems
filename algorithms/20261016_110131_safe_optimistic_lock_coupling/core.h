#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>

#pragma once
#include "types.h"

bool list_init(list_t* list);
bool list_insert(list_t* list, int key, int value);
bool list_remove(list_t* list, int key);
bool list_find(const list_t* list, int key, int* out_value);
