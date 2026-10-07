#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "types.h"
#include "core.h"

static bool compare_factors(const uint32_t *a, size_t alen, const uint32_t *b, size_t blen) {
    if (alen != blen) return false;
    for (size_t i = 0; i < alen; ++i) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

bool are_isomorphic(const Group *g, const Group *h) {
    if (!g || !h) return false;
    return compare_factors(g->factors, g->len, h->factors, h->len);
}

static Element create_generator(const Group *g, size_t idx) {
    Element e;
    e.len = g->len;
    e.coords = (uint32_t *)calloc(e.len, sizeof(uint32_t));
    if (!e.coords) return e;
    e.coords[idx] = 1 % g->factors[idx];
    return e;
}

Basis construct_basis(const Group *g) {
    Basis b;
    b.len = g->len;
    b.vectors = (Element *)calloc(b.len, sizeof(Element));
    if (!b.vectors) {
        b.len = 0;
        return b;
    }
    for (size_t i = 0; i < g->len; ++i) {
        b.vectors[i] = create_generator(g, i);
    }
    return b;
}
