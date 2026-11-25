#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>

#pragma once
#include "types.h"

bool add_terminal_production(Grammar *g, int lhs, char terminal);
bool add_binary_production(Grammar *g, int lhs, int rhs1, int rhs2);
bool cyk_parse(const Grammar *g, const char *input);
