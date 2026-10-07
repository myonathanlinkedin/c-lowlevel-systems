#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>

#pragma once
#include "types.h"

bool are_isomorphic(const Group *g, const Group *h);
Basis construct_basis(const Group *g);
