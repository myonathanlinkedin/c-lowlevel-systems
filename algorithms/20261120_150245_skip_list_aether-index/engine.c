#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/* Simple deterministic xorshift32 PRNG */
static uint32_t rng_next(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

/* Generate a random level for a new node */
static int random_level(SkipList *list) {
    int lvl = 1;
    while (((float)rng_next(&list->rng_state) / (float)UINT32_MAX) < SKIPLIST_P &&
           lvl < SKIPLIST_MAX_LEVEL) {
        ++lvl;
    }
    return lvl;
}

/* Allocate a new node with given level */
static SkipListNode *node_create(int key, int value, int level) {
    SkipListNode *node = (SkipListNode *)malloc(sizeof(SkipListNode));
    if (!node) {
        return NULL;
    }
    node->key = key;
    node->value = value;
    for (int i = 0; i < SKIPLIST_MAX_LEVEL; ++i) {
        node->forward[i] = NULL;
    }
    (void)level; /* level is implicit via forward pointers usage */
    return node;
}

/* Public API implementations */

SkipList *skiplist_create(void) {
    SkipList *list = (SkipList *)malloc(sizeof(SkipList));
    if (!list) {
        return NULL;
    }
    list->level = 1;
    list->size = 0;
    list->rng_state = 0xdeadbeefu; /* deterministic seed */
    list->header = node_create(INT32_MIN, 0, SKIPLIST_MAX_LEVEL);
    if (!list->header) {
        free(list);
        return NULL;
    }
    return list;
}

void skiplist_destroy(SkipList *list) {
    if (!list) return;
    SkipListNode *cur = list->header->forward[0];
    while (cur) {
        SkipListNode *next = cur->forward[0];
        free(cur);
        cur = next;
    }
    free(list->header);
    free(list);
}

bool skiplist_search(const SkipList *list, int key, int *out_value) {
    if (!list) return false;
    const SkipListNode *x = list->header;
    for (int i = list->level - 1; i >= 0; --i) {
        while (x->forward[i] && x->forward[i]->key < key) {
            x = x->forward[i];
        }
    }
    x = x->forward[0];
    if (x && x->key == key) {
        if (out_value) *out_value = x->value;
        return true;
    }
    return false;
}

bool skiplist_insert(SkipList *list, int key, int value) {
    if (!list) return false;
    SkipListNode *update[SKIPLIST_MAX_LEVEL];
    SkipListNode *x = list->header;

    for (int i = list->level - 1; i >= 0; --i) {
        while (x->forward[i] && x->forward[i]->key < key) {
            x = x->forward[i];
        }
        update[i] = x;
    }

    x = x->forward[0];
    if (x && x->key == key) {
        /* Key already exists – update value */
        x->value = value;
        return true;
    } else {
        int lvl = random_level(list);
        if (lvl > list->level) {
            for (int i = list->level; i < lvl; ++i) {
                update[i] = list->header;
            }
            list->level = lvl;
        }
        SkipListNode *new_node = node_create(key, value, lvl);
        if (!new_node) return false;
        for (int i = 0; i < lvl; ++i) {
            new_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = new_node;
        }
        ++list->size;
        return true;
    }
}

bool skiplist_delete(SkipList *list, int key) {
    if (!list) return false;
    SkipListNode *update[SKIPLIST_MAX_LEVEL];
    SkipListNode *x = list->header;

    for (int i = list->level - 1; i >= 0; --i) {
        while (x->forward[i] && x->forward[i]->key < key) {
            x = x->forward[i];
        }
        update[i] = x;
    }

    x = x->forward[0];
    if (!x || x->key != key) {
        return false; /* Not found */
    }

    for (int i = 0; i < list->level; ++i) {
        if (update[i]->forward[i] != x) break;
        update[i]->forward[i] = x->forward[i];
    }
    free(x);
    --list->size;

    while (list->level > 1 && list->header->forward[list->level - 1] == NULL) {
        --list->level;
    }
    return true;
}

size_t skiplist_size(const SkipList *list) {
    return list ? list->size : 0;
}
