#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Simple multiplicative hash; table size is power of two */
static inline size_t hash_uint64(uint64_t x) {
    const uint64_t knuth = 11400714819323198485ULL;
    return (size_t)((x * knuth) >> 32);
}

/* Find entry index; returns table size if not found */
static size_t hash_find(const struct Pager *p, uint64_t page_id) {
    size_t idx = hash_uint64(page_id) & p->table_mask;
    size_t start = idx;
    while (p->table[idx].occupied) {
        if (p->table[idx].page_id == page_id) {
            return idx;
        }
        idx = (idx + 1) & p->table_mask;
        if (idx == start) break; // full loop, not found
    }
    return p->table_mask + 1; // sentinel for not found
}

/* Insert a new mapping; assumes slot exists */
static void hash_insert(struct Pager *p, uint64_t page_id, struct PageFrame *frame) {
    size_t idx = hash_uint64(page_id) & p->table_mask;
    while (p->table[idx].occupied) {
        idx = (idx + 1) & p->table_mask;
    }
    p->table[idx].page_id = page_id;
    p->table[idx].frame = frame;
    p->table[idx].occupied = true;
}

/* Remove mapping; uses lazy deletion (marks unoccupied) */
static void hash_remove(struct Pager *p, uint64_t page_id) {
    size_t idx = hash_find(p, page_id);
    if (idx <= p->table_mask) {
        p->table[idx].occupied = false;
        /* Rehash subsequent cluster */
        size_t next = (idx + 1) & p->table_mask;
        while (p->table[next].occupied) {
            uint64_t move_key = p->table[next].page_id;
            struct PageFrame *move_frame = p->table[next].frame;
            p->table[next].occupied = false;
            hash_insert(p, move_key, move_frame);
            next = (next + 1) & p->table_mask;
        }
    }
}

/* Move frame to front of LRU list */
static void lru_move_to_front(struct Pager *p, struct PageFrame *frame) {
    if (p->lru_head == frame) return; // already front
    // detach
    if (frame->prev) frame->prev->next = frame->next;
    if (frame->next) frame->next->prev = frame->prev;
    if (p->lru_tail == frame) p->lru_tail = frame->prev;
    // insert at head
    frame->prev = NULL;
    frame->next = p->lru_head;
    if (p->lru_head) p->lru_head->prev = frame;
    p->lru_head = frame;
    if (p->lru_tail == NULL) p->lru_tail = frame;
}

/* Evict least-recently-used frame */
static struct PageFrame *lru_evict(struct Pager *p) {
    struct PageFrame *victim = p->lru_tail;
    if (!victim) return NULL;
    // detach tail
    if (victim->prev) {
        victim->prev->next = NULL;
        p->lru_tail = victim->prev;
    } else {
        // only one element
        p->lru_head = p->lru_tail = NULL;
    }
    // remove from hash
    hash_remove(p, victim->page_id);
    return victim;
}

/* Insert new frame as most-recently-used */
static void lru_insert_front(struct Pager *p, struct PageFrame *frame) {
    frame->prev = NULL;
    frame->next = p->lru_head;
    if (p->lru_head) p->lru_head->prev = frame;
    p->lru_head = frame;
    if (p->lru_tail == NULL) p->lru_tail = frame;
}

/* Public API ---------------------------------------------------------- */

void pager_init(struct Pager *p, size_t num_frames) {
    if (!p || num_frames == 0) return;
    p->capacity = num_frames;
    p->size = 0;
    p->hits = 0;
    p->faults = 0;
    p->lru_head = p->lru_tail = NULL;
    p->frames = calloc(num_frames, sizeof(struct PageFrame));
    /* hash table size: next power of two >= 2 * num_frames */
    size_t tbl = 1;
    while (tbl < (num_frames * 2)) tbl <<= 1;
    p->table_mask = tbl - 1;
    p->table = calloc(tbl, sizeof(struct HashEntry));
}

void pager_destroy(struct Pager *p) {
    if (!p) return;
    free(p->frames);
    free(p->table);
    p->frames = NULL;
    p->table = NULL;
    p->capacity = p->size = p->hits = p->faults = 0;
    p->lru_head = p->lru_tail = NULL;
    p->table_mask = 0;
}

/* Access a virtual page; updates LRU and fault/hit counters */
void pager_access(struct Pager *p, uint64_t page_id) {
    if (!p) return;
    size_t idx = hash_find(p, page_id);
    if (idx <= p->table_mask) {
        /* Hit */
        p->hits++;
        struct PageFrame *frame = p->table[idx].frame;
        lru_move_to_front(p, frame);
        return;
    }
    /* Miss -> page fault */
    p->faults++;
    struct PageFrame *frame = NULL;
    if (p->size < p->capacity) {
        /* Allocate unused frame */
        frame = &p->frames[p->size];
        p->size++;
    } else {
        /* Evict LRU */
        frame = lru_evict(p);
    }
    /* Reuse frame for new page */
    frame->page_id = page_id;
    lru_insert_front(p, frame);
    hash_insert(p, page_id, frame);
}

/* Query functions */
size_t pager_get_faults(const struct Pager *p) { return p ? p->faults : 0; }
size_t pager_get_hits(const struct Pager *p)   { return p ? p->hits   : 0; }
size_t pager_get_capacity(const struct Pager *p){ return p ? p->capacity : 0; }
size_t pager_get_loaded(const struct Pager *p) { return p ? p->size : 0; }
