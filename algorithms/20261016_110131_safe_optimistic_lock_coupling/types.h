#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stdatomic.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct node {
    int key;
    int value;
    _Atomic unsigned int version;   // increments on each modification
    _Atomic bool lock;              // simple spin‑lock: false = unlocked, true = locked
    struct node* next;
} node_t;

typedef struct {
    node_t* head;
} list_t;

// ---------------------------------------------------------------------
// Inline lock primitives (must be visible to all translation units)
// ---------------------------------------------------------------------
static inline void lock_node(node_t* n) {
    bool expected = false;
    while (!atomic_compare_exchange_weak(&n->lock, &expected, true)) {
        expected = false;               // reset for next iteration
    }
}
static inline void unlock_node(node_t* n) {
    atomic_store(&n->lock, false);
}
