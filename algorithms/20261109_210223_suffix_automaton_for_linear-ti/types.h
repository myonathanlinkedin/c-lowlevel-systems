
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define SAM_ALPHABET_SIZE 26
#define SAM_CHAR_TO_INDEX(c) ((int)((c) - 'a'))

typedef struct {
    int link;                                 // suffix link
    int len;                                  // length of longest string in this state
    int next[SAM_ALPHABET_SIZE];              // transitions, -1 if absent
} SAMNode;

typedef struct {
    SAMNode *nodes;       // dynamic array of states
    int size;             // number of used states
    int capacity;         // allocated capacity of nodes array
    int last;             // index of the state representing the whole string
} SAM;

/* SAM core API */
void sam_init(SAM *sam);
void sam_extend(SAM *sam, char c);
bool sam_contains(const SAM *sam, const char *s);
void sam_free(SAM *sam);
