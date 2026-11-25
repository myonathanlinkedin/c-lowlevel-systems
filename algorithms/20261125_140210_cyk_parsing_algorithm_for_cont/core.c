#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <string.h>

static uint64_t bitset_add(uint64_t set, int nt) {
    return set | ((uint64_t)1 << (unsigned)nt);
}
static bool bitset_has(uint64_t set, int nt) {
    return (set & ((uint64_t)1 << (unsigned)nt)) != 0;
}

bool add_terminal_production(Grammar *g, int lhs, char terminal) {
    if (g->prod_count >= MAX_PRODUCTIONS || lhs < 0 || lhs >= (int)MAX_NONTERMINALS)
        return false;
    Production *p = &g->prods[g->prod_count++];
    p->lhs = lhs;
    p->terminal = terminal;
    p->is_terminal = true;
    if ((size_t)lhs + 1 > g->nonterm_count)
        g->nonterm_count = (size_t)lhs + 1;
    return true;
}

bool add_binary_production(Grammar *g, int lhs, int rhs1, int rhs2) {
    if (g->prod_count >= MAX_PRODUCTIONS ||
        lhs < 0 || lhs >= (int)MAX_NONTERMINALS ||
        rhs1 < 0 || rhs1 >= (int)MAX_NONTERMINALS ||
        rhs2 < 0 || rhs2 >= (int)MAX_NONTERMINALS)
        return false;
    Production *p = &g->prods[g->prod_count++];
    p->lhs = lhs;
    p->rhs1 = rhs1;
    p->rhs2 = rhs2;
    p->is_terminal = false;
    if ((size_t)lhs + 1 > g->nonterm_count)   g->nonterm_count = (size_t)lhs + 1;
    if ((size_t)rhs1 + 1 > g->nonterm_count) g->nonterm_count = (size_t)rhs1 + 1;
    if ((size_t)rhs2 + 1 > g->nonterm_count) g->nonterm_count = (size_t)rhs2 + 1;
    return true;
}

bool cyk_parse(const Grammar *g, const char *input) {
    size_t n = strlen(input);
    if (n == 0) return false;               // No epsilon handling in this CNF version

    // VLA for the CYK table: each cell holds a bitmask of possible non‑terminals
    uint64_t table[n][n];
    for (size_t i = 0; i < n; ++i)
        for (size_t j = 0; j < n; ++j)
            table[i][j] = 0;

    // Length‑1 substrings (terminals)
    for (size_t i = 0; i < n; ++i) {
        uint64_t cell = 0;
        char a = input[i];
        for (size_t p = 0; p < g->prod_count; ++p) {
            const Production *prod = &g->prods[p];
            if (prod->is_terminal && prod->terminal == a) {
                cell = bitset_add(cell, prod->lhs);
            }
        }
        table[i][0] = cell;
    }

    // Substrings of length >= 2
    for (size_t l = 2; l <= n; ++l) {               // l = span length
        for (size_t i = 0; i <= n - l; ++i) {       // start index
            uint64_t cell = 0;
            for (size_t s = 1; s < l; ++s) {       // split point
                uint64_t left  = table[i][s - 1];
                uint64_t right = table[i + s][l - s - 1];
                if (left == 0 || right == 0) continue;
                for (size_t p = 0; p < g->prod_count; ++p) {
                    const Production *prod = &g->prods[p];
                    if (prod->is_terminal) continue;
                    if (bitset_has(left, prod->rhs1) && bitset_has(right, prod->rhs2)) {
                        cell = bitset_add(cell, prod->lhs);
                    }
                }
            }
            table[i][l - 1] = cell;
        }
    }

    // By convention, start symbol is non‑terminal 0 (S)
    return bitset_has(table[0][n - 1], 0);
}
