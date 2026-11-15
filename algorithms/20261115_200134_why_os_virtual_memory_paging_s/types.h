#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

struct PageFrame {
    uint64_t page_id;               // virtual page identifier
    struct PageFrame *prev;         // LRU previous
    struct PageFrame *next;         // LRU next
};

struct HashEntry {
    uint64_t page_id;
    struct PageFrame *frame;
    bool occupied;
};

struct Pager {
    struct PageFrame *frames;       // array of frames
    struct HashEntry *table;        // hash table for fast lookup
    size_t capacity;                // max number of frames
    size_t size;                    // currently loaded pages
    size_t table_mask;              // (table_size - 1) for fast modulo
    size_t hits;
    size_t faults;
    struct PageFrame *lru_head;     // most recently used
    struct PageFrame *lru_tail;     // least recently used
};
