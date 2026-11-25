#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>

#include "core.h"
#include <assert.h>
#include <stdio.h>

static void test_simple_ab(void) {
    Grammar g = {0};
    // Non‑terminals: 0 = S, 1 = A, 2 = B
    assert(add_terminal_production(&g, 1, 'a'));
    assert(add_terminal_production(&g, 2, 'b'));
    assert(add_binary_production(&g, 0, 1, 2));   // S → A B

    assert(cyk_parse(&g, "ab") == true);
    assert(cyk_parse(&g, "a")  == false);
    assert(cyk_parse(&g, "b")  == false);
    assert(cyk_parse(&g, "aba")== false);
}

static void test_a_n_b_n(void) {
    Grammar g = {0};
    // Non‑terminals: 0 = S, 1 = A, 2 = B
    // A → a , B → b , S → A B , S → S S
    assert(add_terminal_production(&g, 1, 'a'));          // A → a
    assert(add_terminal_production(&g, 2, 'b'));          // B → b
    assert(add_binary_production(&g, 0, 1, 2));           // S → A B
    assert(add_binary_production(&g, 0, 0, 0));           // S → S S (recursion)

    // Accepted strings
    assert(cyk_parse(&g, "ab")      == true);
    assert(cyk_parse(&g, "aabb")    == true);
    assert(cyk_parse(&g, "aaabbb")  == true);
    assert(cyk_parse(&g, "aaaabbbb")== true);

    // Rejected strings
    assert(cyk_parse(&g, "aab")   == false);
    assert(cyk_parse(&g, "abb")   == false);
    assert(cyk_parse(&g, "aabbb") == false);
    assert(cyk_parse(&g, "")      == false);
}

int main(void) {
    test_simple_ab();
    test_a_n_b_n();
    printf("All CYK tests passed.\n");
    return 0;
}
