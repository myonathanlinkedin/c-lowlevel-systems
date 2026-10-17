#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>

#include "core.h"
#include <stdlib.h>
#include <assert.h>

// Global lock protecting modifications of the list head pointer
static _Atomic bool head_lock = false;

// ---------------------------------------------------------------------
// Helper: locate predecessor of the node that should contain `key`.
// Returns predecessor (may be NULL) and stores the current node in *out_cur.
// ---------------------------------------------------------------------
static node_t* find_predecessor(list_t* list, int key, node_t** out_cur) {
    node_t* prev = NULL;
    node_t* cur  = list->head;
    while (cur && cur->key < key) {
        prev = cur;
        cur  = cur->next;
    }
    if (out_cur) *out_cur = cur;
    return prev;
}

// ---------------------------------------------------------------------
// Initialise an empty list.
// ---------------------------------------------------------------------
bool list_init(list_t* list) {
    if (!list) return false;
    list->head = NULL;
    return true;
}

// ---------------------------------------------------------------------
// Insert a key/value pair. If the key already exists its value is updated.
// ---------------------------------------------------------------------
bool list_insert(list_t* list, int key, int value) {
    if (!list) return false;

    node_t* cur = NULL;
    node_t* pred = find_predecessor(list, key, &cur);

    // Key already present – update under node lock
    if (cur && cur->key == key) {
        lock_node(cur);
        cur->value = value;
        unsigned int ver = atomic_load(&cur->version);
        atomic_store(&cur->version, ver + 1);
        unlock_node(cur);
        return true;
    }

    // Allocate and initialise the new node
    node_t* new_node = (node_t*)malloc(sizeof(node_t));
    if (!new_node) return false;
    new_node->key = key;
    new_node->value = value;
    atomic_init(&new_node->version, 0);
    atomic_init(&new_node->lock, false);
    new_node->next = cur;

    // Insert at head (no predecessor)
    if (!pred) {
        bool expected = false;
        while (!atomic_compare_exchange_weak(&head_lock, &expected, true)) {
            expected = false;
        }
        // Validate that head has not changed since we observed it
        if (list->head != cur) {
            atomic_store(&head_lock, false);
            free(new_node);
            return false;
        }
        list->head = new_node;
        atomic_store(&head_lock, false);
        return true;
    }

    // Insert after predecessor – lock predecessor first
    lock_node(pred);
    // Validate that predecessor still points to the same successor
    unsigned int pred_ver = atomic_load(&pred->version);
    if (pred->next != cur) {
        unlock_node(pred);
        free(new_node);
        return false;   // concurrent modification detected
    }
    pred->next = new_node;
    atomic_store(&pred->version, pred_ver + 1);
    unlock_node(pred);
    return true;
}

// ---------------------------------------------------------------------
// Remove a node identified by `key`.
// ---------------------------------------------------------------------
bool list_remove(list_t* list, int key) {
    if (!list) return false;

    node_t* cur = NULL;
    node_t* pred = find_predecessor(list, key, &cur);
    if (!cur || cur->key != key) return false;   // not found

    // Lock nodes needed for safe unlinking
    if (pred) lock_node(pred);
    lock_node(cur);

    // Verify that the list structure has not changed while acquiring locks
    if ((pred && pred->next != cur) ||
        (!pred && list->head != cur)) {
        unlock_node(cur);
        if (pred) unlock_node(pred);
        return false;
    }

    // Unlink the node
    if (pred) {
        pred->next = cur->next;
        unsigned int pred_ver = atomic_load(&pred->version);
        atomic_store(&pred->version, pred_ver + 1);
        unlock_node(pred);
    } else {
        // Removing the head – protect the head pointer with the global lock
        bool expected = false;
        while (!atomic_compare_exchange_weak(&head_lock, &expected, true)) {
            expected = false;
        }
        list->head = cur->next;
        atomic_store(&head_lock, false);
    }

    // Bump version of the removed node before freeing (helps optimistic readers)
    unsigned int cur_ver = atomic_load(&cur->version);
    atomic_store(&cur->version, cur_ver + 1);
    unlock_node(cur);
    free(cur);
    return true;
}

// ---------------------------------------------------------------------
// Optimistic read: locate `key` without acquiring locks, then validate.
// ---------------------------------------------------------------------
bool list_find(const list_t* list, int key, int* out_value) {
    if (!list) return false;

    node_t* cur = list->head;
    while (cur && cur->key < key) {
        cur = cur->next;
    }
    if (!cur || cur->key != key) return false;   // not present

    // Snapshot version and lock state before reading the value
    unsigned int ver_before = atomic_load(&cur->version);
    if (atomic_load(&cur->lock)) return false;   // writer in progress

    int value = cur->value;                       // read protected data

    // Verify that version has not changed after the read
    unsigned int ver_after = atomic_load(&cur->version);
    if (ver_before != ver_after) return false;   // concurrent modification

    if (out_value) *out_value = value;
    return true;
}
