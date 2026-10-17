#include "types.h"
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define CACHE_CAPACITY 1021  /* a prime number for hash distribution */

static InternNode **table = NULL;
static size_t capacity = CACHE_CAPACITY;
static bool initialized = false;

/* Simple djb2 hash, cast shift count to unsigned */
static uint32_t hash_string(const char *s) {
    uint32_t h = 5381;
    unsigned char c;
    while ((c = (unsigned char)*s++) != '\0') {
        h = ((h << 5) + h) + c; /* h * 33 + c */
    }
    return h;
}

/* Ensure the hash table is allocated */
static void ensure_init(void) {
    if (!initialized) {
        table = (InternNode **)calloc(capacity, sizeof(InternNode *));
        if (table != NULL) {
            initialized = true;
        }
    }
}

/* Duplicate a string using malloc */
static char *dup_string(const char *s) {
    size_t len = strlen(s);
    char *copy = (char *)malloc(len + 1);
    if (copy) {
        memcpy(copy, s, len + 1);
    }
    return copy;
}

/* Insert a new node at the head of the bucket list */
static InternNode *node_create(const char *s) {
    InternNode *n = (InternNode *)malloc(sizeof(InternNode));
    if (!n) return NULL;
    n->str = dup_string(s);
    if (!n->str) {
        free(n);
        return NULL;
    }
    n->next = NULL;
    return n;
}

/* Public API */
const char *cache_command(const char *cmd) {
    if (cmd == NULL) return NULL;
    ensure_init();
    if (!initialized) return NULL;  /* allocation failure */

    uint32_t h = hash_string(cmd);
    size_t idx = (size_t)(h % (uint32_t)capacity);
    InternNode *cur = table[idx];
    while (cur) {
        if (strcmp(cur->str, cmd) == 0) {
            return cur->str;  /* already cached */
        }
        cur = cur->next;
    }
    /* Not found – create new node */
    InternNode *new_node = node_create(cmd);
    if (!new_node) return NULL;  /* allocation failure */
    new_node->next = table[idx];
    table[idx] = new_node;
    return new_node->str;
}

void cache_free(void) {
    if (!initialized) return;
    for (size_t i = 0; i < capacity; ++i) {
        InternNode *cur = table[i];
        while (cur) {
            InternNode *next = cur->next;
            free(cur->str);
            free(cur);
            cur = next;
        }
    }
    free(table);
    table = NULL;
    initialized = false;
}
