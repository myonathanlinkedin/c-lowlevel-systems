
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#pragma once

#include <stddef.h>
#include <stdbool.h>

/* Maximum number of children per trie node.
   Supports lowercase English letters 'a'..'z'. */
#define TRIE_ALPHABET_SIZE 26

/* Trie node definition */
typedef struct TrieNode {
    struct TrieNode *children[TRIE_ALPHABET_SIZE];
    bool is_end;                     /* true if node terminates a word */
    unsigned int frequency;          /* frequency of the word ending here */
} TrieNode;

/* Structure used to hold a word and its frequency during collection */
typedef struct {
    char *word;                      /* dynamically allocated null‑terminated string */
    unsigned int frequency;
} TrieResult;

/* Result container for autocomplete */
typedef struct {
    TrieResult *items;               /* dynamically allocated array of results */
    size_t count;                    /* number of valid items in the array */
    size_t capacity;                 /* allocated capacity of the array */
} TrieResultList;
