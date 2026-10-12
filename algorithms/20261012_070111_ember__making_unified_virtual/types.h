#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define PAGE_SIZE 4096U
#define MAX_VIRTUAL_PAGES 1024U
#define MAX_PHYSICAL_PAGES 256U

typedef struct {
    bool valid;               // Is the virtual page mapped
    bool dirty;               // Has the page been written
    size_t phys_page;         // Index into physical memory array
    uint64_t last_used;       // Timestamp for LRU
} PageTableEntry;

typedef struct {
    bool occupied;            // Is the physical page in use
    uint8_t data[PAGE_SIZE];  // Page contents
} PhysicalPage;

typedef struct {
    PageTableEntry page_table[MAX_VIRTUAL_PAGES];
    PhysicalPage phys_mem[MAX_PHYSICAL_PAGES];
    uint8_t swap_storage[MAX_VIRTUAL_PAGES][PAGE_SIZE]; // Simulated swap/backing store
    size_t next_vpage;        // Next free virtual page index for allocation
    uint64_t tick;            // Global logical clock for LRU
} VirtualMemoryManager;
