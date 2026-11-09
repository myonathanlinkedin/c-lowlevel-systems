#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include "types.h"

static void sam_ensure_capacity(SAM *sam) {
    if (sam->size < sam->capacity) {
        return;
    }
    int new_cap = (sam->capacity == 0) ? 4 : sam->capacity * 2;
    SAMNode *new_buf = (SAMNode *)realloc(sam->nodes, new_cap * sizeof(SAMNode));
    assert(new_buf != NULL);
    sam->nodes = new_buf;
    sam->capacity = new_cap;
}

/* Initialize an empty suffix automaton */
void sam_init(SAM *sam) {
    sam->nodes = NULL;
    sam->size = 0;
    sam->capacity = 0;
    sam->last = -1;
    sam_ensure_capacity(sam);
    SAMNode *root = &sam->nodes[0];
    root->link = -1;
    root->len = 0;
    for (int i = 0; i < SAM_ALPHABET_SIZE; ++i) {
        root->next[i] = -1;
    }
    sam->size = 1;
    sam->last = 0;
}

/* Extend the automaton with character c (assumed 'a'..'z') */
void sam_extend(SAM *sam, char c) {
    int cur_char = SAM_CHAR_TO_INDEX(c);
    assert(cur_char >= 0 && cur_char < SAM_ALPHABET_SIZE);

    sam_ensure_capacity(sam);
    SAMNode *cur = &sam->nodes[sam->size];
    cur->len = sam->nodes[sam->last].len + 1;
    for (int i = 0; i < SAM_ALPHABET_SIZE; ++i) {
        cur->next[i] = -1;
    }
    cur->link = 0; // temporary, will be fixed later
    int cur_idx = sam->size;
    sam->size++;

    int p = sam->last;
    while (p != -1 && sam->nodes[p].next[cur_char] == -1) {
        sam->nodes[p].next[cur_char] = cur_idx;
        p = sam->nodes[p].link;
    }

    if (p == -1) {
        cur->link = 0;
    } else {
        int q = sam->nodes[p].next[cur_char];
        if (sam->nodes[p].len + 1 == sam->nodes[q].len) {
            cur->link = q;
        } else {
            /* Clone state q */
            sam_ensure_capacity(sam);
            SAMNode *clone = &sam->nodes[sam->size];
            *clone = sam->nodes[q];               // copy all fields
            clone->len = sam->nodes[p].len + 1;
            int clone_idx = sam->size;
            sam->size++;

            while (p != -1 && sam->nodes[p].next[cur_char] == q) {
                sam->nodes[p].next[cur_char] = clone_idx;
                p = sam->nodes[p].link;
            }
            sam->nodes[q].link = clone_idx;
            cur->link = clone_idx;
        }
    }
    sam->last = cur_idx;
}

/* Check whether string s occurs as a substring */
bool sam_contains(const SAM *sam, const char *s) {
    if (s == NULL) {
        return false;
    }
    int v = 0; // start at root
    for (size_t i = 0; s[i] != '\0'; ++i) {
        int ch = SAM_CHAR_TO_INDEX(s[i]);
        if (ch < 0 || ch >= SAM_ALPHABET_SIZE) {
            return false; // character outside alphabet
        }
        int nxt = sam->nodes[v].next[ch];
        if (nxt == -1) {
            return false;
        }
        v = nxt;
    }
    return true;
}

/* Release allocated memory */
void sam_free(SAM *sam) {
    free(sam->nodes);
    sam->nodes = NULL;
    sam->size = 0;
    sam->capacity = 0;
    sam->last = -1;
}
