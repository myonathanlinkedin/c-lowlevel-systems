#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_NONTERMINALS 64
#define MAX_PRODUCTIONS 256

typedef struct {
    int lhs;            // left‑hand side non‑terminal index
    int rhs1;           // first right‑hand side symbol (non‑terminal)
    int rhs2;           // second right‑hand side symbol (non‑terminal)
    bool is_terminal;  // true if production is A → a
    char terminal;     // terminal symbol for A → a
} Production;

typedef struct {
    Production prods[MAX_PRODUCTIONS];
    size_t prod_count;
    size_t nonterm_count;   // highest non‑terminal index + 1
} Grammar;
