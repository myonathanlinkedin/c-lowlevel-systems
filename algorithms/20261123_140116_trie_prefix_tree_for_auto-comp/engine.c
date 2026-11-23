#include "types.h"
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ---------- Utility Functions ---------- */

static inline int char_to_index(char c) {
    /* Accept only lowercase a‑z; other characters map to -1 */
    if (c >= 'a' && c <= 'z')
        return (int)(c - 'a');
    return -1;
}

/* Create a new trie node; all fields zero‑initialized. */
static TrieNode *trie_node_create(void) {
    TrieNode *node = (TrieNode *)calloc(1, sizeof(TrieNode));
    return node;
}

/* ---------- Public API ---------- */

TrieNode *trie_create(void) {
    return trie_node_create();
}

static void trie_node_destroy(TrieNode *node) {
    if (!node) return;
    for (int i = 0; i < TRIE_ALPHABET_SIZE; ++i) {
        if (node->children[i])
            trie_node_destroy(node->children[i]);
    }
    free(node);
}

void trie_destroy(TrieNode *root) {
    trie_node_destroy(root);
}

void trie_insert(TrieNode *root, const char *word, unsigned int frequency) {
    assert(root != NULL);
    assert(word != NULL);

    TrieNode *cur = root;
    for (size_t i = 0; word[i] != '\0'; ++i) {
        int idx = char_to_index(word[i]);
        assert(idx >= 0);                     /* enforce lowercase a‑z */
        if (!cur->children[idx]) {
            cur->children[idx] = trie_node_create();
            assert(cur->children[idx] != NULL);
        }
        cur = cur->children[idx];
    }
    cur->is_end = true;
    cur->frequency = frequency;
}

void trie_increment(TrieNode *root, const char *word) {
    assert(root != NULL);
    assert(word != NULL);

    TrieNode *cur = root;
    for (size_t i = 0; word[i] != '\0'; ++i) {
        int idx = char_to_index(word[i]);
        assert(idx >= 0);
        if (!cur->children[idx]) {
            cur->children[idx] = trie_node_create();
            assert(cur->children[idx] != NULL);
        }
        cur = cur->children[idx];
    }
    cur->is_end = true;
    cur->frequency += 1;
}

/* ---------- Result List Management ---------- */

static void result_list_init(TrieResultList *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

static void result_list_reserve(TrieResultList *list, size_t new_cap) {
    if (new_cap <= list->capacity) return;
    size_t cap = list->capacity ? list->capacity * 2 : 8;
    while (cap < new_cap) cap *= 2;
    TrieResult *new_items = (TrieResult *)realloc(list->items, cap * sizeof(TrieResult));
    assert(new_items != NULL);
    list->items = new_items;
    list->capacity = cap;
}

static void result_list_push(TrieResultList *list, const char *word, unsigned int freq) {
    result_list_reserve(list, list->count + 1);
    list->items[list->count].word = strdup(word);
    assert(list->items[list->count].word != NULL);
    list->items[list->count].frequency = freq;
    list->count += 1;
}

/* ---------- DFS Collection ---------- */

void _trie_collect_all(const TrieNode *node,
                       char *buffer,
                       size_t depth,
                       TrieResultList *list) {
    if (!node) return;
    if (node->is_end) {
        buffer[depth] = '\0';
        result_list_push(list, buffer, node->frequency);
    }
    for (int i = 0; i < TRIE_ALPHABET_SIZE; ++i) {
        if (node->children[i]) {
            buffer[depth] = (char)('a' + i);
            _trie_collect_all(node->children[i], buffer, depth + 1, list);
        }
    }
}

/* ---------- Sorting ---------- */

static int compare_results(const void *a, const void *b) {
    const TrieResult *ra = (const TrieResult *)a;
    const TrieResult *rb = (const TrieResult *)b;
    if (ra->frequency != rb->frequency)
        return (rb->frequency > ra->frequency) - (rb->frequency < ra->frequency); /* descending */
    return strcmp(ra->word, rb->word);                                            /* ascending */
}

/* ---------- Autocomplete ---------- */

void trie_autocomplete(const TrieNode *root,
                       const char *prefix,
                       size_t max_results,
                       char ***out_words,
                       size_t *out_count) {
    assert(root != NULL);
    assert(prefix != NULL);
    assert(out_words != NULL);
    assert(out_count != NULL);

    const TrieNode *cur = root;
    for (size_t i = 0; prefix[i] != '\0'; ++i) {
        int idx = char_to_index(prefix[i]);
        assert(idx >= 0);
        if (!cur->children[idx]) {
            /* No completions */
            *out_words = NULL;
            *out_count = 0;
            return;
        }
        cur = cur->children[idx];
    }

    TrieResultList list;
    result_list_init(&list);
    char buffer[256];                     /* sufficient for typical words */
    strcpy(buffer, prefix);
    size_t prefix_len = strlen(prefix);
    _trie_collect_all(cur, buffer, prefix_len, &list);

    if (list.count == 0) {
        *out_words = NULL;
        *out_count = 0;
        free(list.items);
        return;
    }

    qsort(list.items, list.count, sizeof(TrieResult), compare_results);

    size_t to_return = (max_results && max_results < list.count) ? max_results : list.count;
    char **result_array = (char **)malloc(to_return * sizeof(char *));
    assert(result_array != NULL);
    for (size_t i = 0; i < to_return; ++i) {
        result_array[i] = list.items[i].word;   /* transfer ownership */
    }

    /* Free any surplus items */
    for (size_t i = to_return; i < list.count; ++i) {
        free(list.items[i].word);
    }
    free(list.items);

    *out_words = result_array;
    *out_count = to_return;
}
