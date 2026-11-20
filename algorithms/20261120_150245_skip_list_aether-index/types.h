
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/* Configuration constants */
#define SKIPLIST_MAX_LEVEL 16      /* Maximum level for the skip list */
#define SKIPLIST_P 0.5f            /* Probability factor for level generation */

/* Forward declaration */
struct SkipListNode;

/* Skip list node structure */
typedef struct SkipListNode {
    int key;                                 /* Key stored in the node */
    int value;                               /* Associated value */
    struct SkipListNode *forward[SKIPLIST_MAX_LEVEL]; /* Forward pointers per level */
} SkipListNode;

/* Skip list structure */
typedef struct SkipList {
    int level;                /* Current highest level of the list */
    size_t size;              /* Number of elements stored */
    SkipListNode *header;     /* Header sentinel node */
    uint32_t rng_state;       /* State for deterministic pseudo‑random generator */
} SkipList;

/* Public API */
SkipList *skiplist_create(void);
void skiplist_destroy(SkipList *list);
bool skiplist_insert(SkipList *list, int key, int value);
bool skiplist_delete(SkipList *list, int key);
bool skiplist_search(const SkipList *list, int key, int *out_value);
size_t skiplist_size(const SkipList *list);
