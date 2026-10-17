#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>

#include "types.h"
#include "core.h"
#include <stdlib.h>
#include <assert.h>

/* ---------- internal lock primitives ---------- */
static inline void lock_node(Node *n) {
    unsigned int expected;
    for (;;) {
        expected = atomic_load_explicit(&n->lock, memory_order_acquire);
        if ((expected & 1U) == 0) {                     // unlocked (even)
            if (atomic_compare_exchange_weak_explicit(
                    &n->lock, &expected, expected + 1,
                    memory_order_acquire, memory_order_relaxed)) {
                return;                                 // acquired lock (odd)
            }
        } else {
            /* busy‑wait; in production code a pause/yield would be used */
        }
    }
}

static inline void unlock_node(Node *n) {
    /* Incrementing an odd version makes it even again → unlocked */
    atomic_fetch_add_explicit(&n->lock, 1U, memory_order_release);
}

/* ---------- list management ---------- */
static Node *node_new(int key, int value) {
    Node *n = (Node *)malloc(sizeof(Node));
    if (!n) return NULL;
    n->key   = key;
    n->value = value;
    atomic_init(&n->lock, 0U);   // start unlocked
    n->next  = NULL;
    return n;
}

List *list_create(void) {
    List *list = (List *)malloc(sizeof(List));
    if (!list) return NULL;
    Node *sentinel = node_new(INT_MIN, 0);
    if (!sentinel) {
        free(list);
        return NULL;
    }
    list->head = sentinel;
    return list;
}

void list_destroy(List *list) {
    if (!list) return;
    Node *cur = list->head;
    while (cur) {
        Node *next = cur->next;
        free(cur);
        cur = next;
    }
    free(list);
}

/* ---------- optimistic traversal helpers ---------- */
static bool locate(const List *list, int key, Node **out_prev, Node **out_curr) {
    retry:
    Node *prev = list->head;
    Node *curr = prev->next;

    unsigned int prev_ver = atomic_load_explicit(&prev->lock, memory_order_acquire);
    unsigned int curr_ver = curr ? atomic_load_explicit(&curr->lock, memory_order_acquire) : 0U;

    while (curr && curr->key < key) {
        prev = curr;
        prev_ver = curr_ver;
        curr = curr->next;
        curr_ver = curr ? atomic_load_explicit(&curr->lock, memory_order_acquire) : 0U;
    }

    /* Validate that neither node changed while we traversed */
    if ((prev_ver & 1U) || (curr && (curr_ver & 1U))) {
        goto retry;   // a writer held a lock; restart
    }
    if (prev->next != curr) {
        goto retry;   // structural change detected
    }

    *out_prev = prev;
    *out_curr = curr;
    return true;
}

/* ---------- public API ---------- */
bool list_search(const List *list, int key, int *out_value) {
    Node *prev, *curr;
    if (!locate(list, key, &prev, &curr))
        return false;   // should never happen; locate always succeeds

    if (curr && curr->key == key) {
        if (out_value) *out_value = curr->value;
        return true;
    }
    return false;
}

bool list_insert(List *list, int key, int value) {
    Node *prev, *curr;
    if (!locate(list, key, &prev, &curr))
        return false;

    if (curr && curr->key == key) {
        return false;   // duplicate key
    }

    /* Acquire locks in address order to avoid deadlock */
    if (prev < curr) {
        lock_node(prev);
        if (curr) lock_node(curr);
    } else {
        if (curr) lock_node(curr);
        lock_node(prev);
    }

    /* Re‑validate after acquiring locks */
    if (prev->next != curr) {
        unlock_node(prev);
        if (curr) unlock_node(curr);
        return false;   // retry by caller
    }

    Node *new_node = node_new(key, value);
    if (!new_node) {
        unlock_node(prev);
        if (curr) unlock_node(curr);
        return false;   // OOM
    }

    new_node->next = curr;
    prev->next = new_node;

    unlock_node(prev);
    if (curr) unlock_node(curr);
    return true;
}

bool list_delete(List *list, int key) {
    Node *prev, *curr;
    if (!locate(list, key, &prev, &curr))
        return false;

    if (!curr || curr->key != key) {
        return false;   // not found
    }

    /* Acquire locks in address order */
    if (prev < curr) {
        lock_node(prev);
        lock_node(curr);
    } else {
        lock_node(curr);
        lock_node(prev);
    }

    /* Re‑validate */
    if (prev->next != curr) {
        unlock_node(prev);
        unlock_node(curr);
        return false;   // retry by caller
    }

    prev->next = curr->next;
    unlock_node(prev);
    unlock_node(curr);
    free(curr);
    return true;
}
