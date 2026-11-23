#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#pragma once

#include "types.h"

/* Create an empty trie. Returns NULL on allocation failure. */
TrieNode *trie_create(void);

/* Recursively free all nodes of the trie. */
void trie_destroy(TrieNode *root);

/* Insert a word with an associated frequency.
   If the word already exists, its frequency is set to the supplied value. */
void trie_insert(TrieNode *root, const char *word, unsigned int frequency);

/* Increment the frequency of a word by one.
   If the word does not exist, it is inserted with frequency 1. */
void trie_increment(TrieNode *root, const char *word);

/* Autocomplete: given a prefix, retrieve up to max_results completions
   ordered by descending frequency (ties broken by lexicographic order).
   The caller receives a dynamically allocated array of strings via out_words
   and the number of returned entries via out_count.
   The caller must free each string and the array itself. */
void trie_autocomplete(const TrieNode *root,
                       const char *prefix,
                       size_t max_results,
                       char ***out_words,
                       size_t *out_count);

/* Internal helper used by unit tests to verify the internal result list.
   Not part of the public API. */
void _trie_collect_all(const TrieNode *node,
                       char *buffer,
                       size_t depth,
                       TrieResultList *list);
