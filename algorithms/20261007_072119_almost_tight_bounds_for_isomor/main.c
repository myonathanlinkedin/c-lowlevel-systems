#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "types.h"
#include "core.h"

static void free_basis(Basis *b) {
    if (!b) return;
    for (size_t i = 0; i < b->len; ++i) {
        free(b->vectors[i].coords);
    }
    free(b->vectors);
    b->len = 0;
}

static Group make_group(const uint32_t *factors, size_t len) {
    Group g;
    g.len = len;
    g.factors = (uint32_t *)malloc(len * sizeof(uint32_t));
    memcpy(g.factors, factors, len * sizeof(uint32_t));
    return g;
}

static void free_group(Group *g) {
    free(g->factors);
    g->len = 0;
}

static void test_isomorphism(void) {
    uint32_t f1[] = {2, 4, 8};
    uint32_t f2[] = {2, 4, 8};
    uint32_t f3[] = {2, 4, 16};

    Group g1 = make_group(f1, 3);
    Group g2 = make_group(f2, 3);
    Group g3 = make_group(f3, 3);

    assert(are_isomorphic(&g1, &g2));
    assert(!are_isomorphic(&g1, &g3));

    free_group(&g1);
    free_group(&g2);
    free_group(&g3);
}

static void test_basis_construction(void) {
    uint32_t f[] = {3, 5, 7};
    Group g = make_group(f, 3);
    Basis b = construct_basis(&g);

    assert(b.len == g.len);
    for (size_t i = 0; i < b.len; ++i) {
        Element e = b.vectors[i];
        assert(e.len == g.len);
        for (size_t j = 0; j < e.len; ++j) {
            if (j == i) {
                assert(e.coords[j] == 1 % f[j]);
            } else {
                assert(e.coords[j] == 0);
            }
        }
    }

    free_basis(&b);
    free_group(&g);
}

int main(void) {
    test_isomorphism();
    test_basis_construction();
    printf("All tests passed.\n");
    return 0;
}
