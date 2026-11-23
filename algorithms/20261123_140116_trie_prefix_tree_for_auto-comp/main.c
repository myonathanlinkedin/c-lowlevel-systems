#include "types.h"
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#include "core.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

/* Helper to free autocomplete results */
static void free_autocomplete(char **words, size_t count) {
    if (!words) return;
    for (size_t i = 0; i < count; ++i) {
        free(words[i]);
    }
    free(words);
}

/* Unit test: basic insertion and retrieval */
static void test_basic(void) {
    TrieNode *trie = trie_create();
    assert(trie != NULL);

    trie_insert(trie, "apple", 5);
    trie_insert(trie, "app", 3);
    trie_insert(trie, "application", 2);
    trie_insert(trie, "banana", 4);
    trie_insert(trie, "band", 1);
    trie_insert(trie, "bandana", 6);
    trie_insert(trie, "cat", 2);
    trie_insert(trie, "cater", 1);
    trie_insert(trie, "dog", 7);

    /* Autocomplete for prefix "app" */
    char **words = NULL;
    size_t count = 0;
    trie_autocomplete(trie, "app", 10, &words, &count);
    assert(count == 3);
    assert(strcmp(words[0], "apple") == 0);        /* freq 5 */
    assert(strcmp(words[1], "app") == 0);          /* freq 3 */
    assert(strcmp(words[2], "application") == 0); /* freq 2 */
    free_autocomplete(words, count);

    /* Autocomplete for prefix "ba" with max 2 results */
    trie_autocomplete(trie, "ba", 2, &words, &count);
    assert(count == 2);
    assert(strcmp(words[0], "bandana") == 0); /* freq 6 */
    assert(strcmp(words[1], "banana") == 0);  /* freq 4 */
    free_autocomplete(words, count);

    /* Autocomplete for empty prefix (all words) limited to 5 */
    trie_autocomplete(trie, "", 5, &words, &count);
    assert(count == 5);
    /* Expected top 5 frequencies: dog(7), bandana(6), apple(5), banana(4), app(3) */
    const char *expected[5] = {"dog", "bandana", "apple", "banana", "app"};
    for (size_t i = 0; i < 5; ++i) {
        assert(strcmp(words[i], expected[i]) == 0);
    }
    free_autocomplete(words, count);

    /* Increment frequency of an existing word and verify ranking changes */
    trie_increment(trie, "app");   /* app frequency becomes 4 */
    trie_autocomplete(trie, "app", 10, &words, &count);
    assert(count == 3);
    assert(strcmp(words[0], "apple") == 0);        /* still 5 */
    assert(strcmp(words[1], "app") == 0);          /* now 4 */
    assert(strcmp(words[2], "application") == 0); /* 2 */
    free_autocomplete(words, count);

    /* Prefix with no matches */
    trie_autocomplete(trie, "z", 5, &words, &count);
    assert(count == 0);
    assert(words == NULL);

    trie_destroy(trie);
}

/* Unit test: edge cases – long words, maximum depth, and full alphabet */
static void test_edge_cases(void) {
    TrieNode *trie = trie_create();
    assert(trie != NULL);

    /* Long word (250 characters) */
    char long_word[251];
    for (int i = 0; i < 250; ++i) long_word[i] = 'a' + (i % 26);
    long_word[250] = '\0';
    trie_insert(trie, long_word, 1);

    char **words = NULL;
    size_t count = 0;
    trie_autocomplete(trie, long_word, 1, &words, &count);
    assert(count == 1);
    assert(strcmp(words[0], long_word) == 0);
    free_autocomplete(words, count);

    /* Insert all single‑letter words with varying frequencies */
    for (char c = 'a'; c <= 'z'; ++c) {
        char s[2] = {c, '\0'};
        trie_insert(trie, s, (unsigned int)(c - 'a' + 1));
    }

    trie_autocomplete(trie, "", 26, &words, &count);
    assert(count == 26);
    /* Verify that 'z' (freq 26) is first */
    assert(strcmp(words[0], "z") == 0);
    assert(strcmp(words[25], "a") == 0);
    free_autocomplete(words, count);

    trie_destroy(trie);
}

/* Main driver */
int main(void) {
    test_basic();
    test_edge_cases();

    printf("All trie prefix‑tree tests passed.\n");
    return 0;
}
