#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include <stdio.h>
#include "types.h"

static void test_basic(void) {
    SAM sam;
    sam_init(&sam);
    const char *text = "ababa";
    for (size_t i = 0; text[i] != '\0'; ++i) {
        sam_extend(&sam, text[i]);
    }

    /* Positive cases */
    assert(sam_contains(&sam, "a"));
    assert(sam_contains(&sam, "b"));
    assert(sam_contains(&sam, "ab"));
    assert(sam_contains(&sam, "ba"));
    assert(sam_contains(&sam, "aba"));
    assert(sam_contains(&sam, "bab"));
    assert(sam_contains(&sam, "ababa"));
    assert(sam_contains(&sam, ""));   // empty substring is always present

    /* Negative cases */
    assert(!sam_contains(&sam, "aa"));
    assert(!sam_contains(&sam, "c"));
    assert(!sam_contains(&sam, "abb"));
    assert(!sam_contains(&sam, "babaac"));

    sam_free(&sam);
}

static void test_random(void) {
    SAM sam;
    sam_init(&sam);
    const char *text = "abcdefghijklmnopqrstuvwxyz";
    for (size_t i = 0; text[i] != '\0'; ++i) {
        sam_extend(&sam, text[i]);
    }

    /* All single letters should be present */
    for (char c = 'a'; c <= 'z'; ++c) {
        char buf[2] = {c, '\0'};
        assert(sam_contains(&sam, buf));
    }

    /* Substrings of length 2 */
    for (char c = 'a'; c < 'z'; ++c) {
        char buf[3] = {c, (char)(c + 1), '\0'};
        assert(sam_contains(&sam, buf));
    }

    /* Non‑existent substrings */
    assert(!sam_contains(&sam, "aa"));
    assert(!sam_contains(&sam, "az"));
    assert(!sam_contains(&sam, "za"));
    assert(!sam_contains(&sam, "abcdeff"));

    sam_free(&sam);
}

int main(void) {
    test_basic();
    test_random();
    printf("All suffix automaton tests passed.\n");
    return 0;
}
