#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once
#include "types.h"

rc_string *rc_string_new(const char *init);
rc_string *rc_string_retain(rc_string *s);
void rc_string_release(rc_string *s);
const char *rc_string_get(const rc_string *s);
size_t rc_string_refcount(const rc_string *s);
