#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static uint8_t* g_pool = NULL;          // Raw memory pool
static size_t   g_pool_size = 0;        // Total size of the pool
static BlockHeader* g_head = NULL;     // First block in memory order
static BlockHeader* g_free_head = NULL; // Head of free list

// Helper: insert a free block into the free list (LIFO)
static void free_list_insert(BlockHeader* block) {
    block->next_free = g_free_head;
    block->prev_free = NULL;
    if (g_free_head) {
        g_free_head->prev_free = block;
    }
    g_free_head = block;
}

// Helper: remove a free block from the free list
static void free_list_remove(BlockHeader* block) {
    if (block->prev_free) {
        block->prev_free->next_free = block->next_free;
    } else {
        g_free_head = block->next_free;
    }
    if (block->next_free) {
        block->next_free->prev_free = block->prev_free;
    }
    block->next_free = block->prev_free = NULL;
}

// Helper: split a block if it's significantly larger than requested size.
static void split_block(BlockHeader* block, size_t req_size) {
    // Minimum block size to remain useful: header + alignment
    const size_t min_block = ALIGN_UP(sizeof(BlockHeader)) + ALIGNMENT;
    if (block->size >= req_size + min_block) {
        // Create new block after the allocated region
        uint8_t* new_block_addr = (uint8_t*)block + sizeof(BlockHeader) + req_size;
        BlockHeader* new_block = (BlockHeader*)new_block_addr;
        new_block->size = block->size - req_size - sizeof(BlockHeader);
        new_block->free = true;
        // Insert into memory order list
        new_block->next = block->next;
        new_block->prev = block;
        if (block->next) block->next->prev = new_block;
        block->next = new_block;
        // Adjust original block size
        block->size = req_size;
        // Add new block to free list
        free_list_insert(new_block);
    }
}

// Helper: coalesce adjacent free blocks
static void coalesce(BlockHeader* block) {
    // Merge with previous if free
    if (block->prev && block->prev->free) {
        BlockHeader* prev = block->prev;
        prev->size += sizeof(BlockHeader) + block->size;
        prev->next = block->next;
        if (block->next) block->next->prev = prev;
        free_list_remove(block);
        block = prev;
    }
    // Merge with next if free
    if (block->next && block->next->free) {
        BlockHeader* next = block->next;
        block->size += sizeof(BlockHeader) + next->size;
        block->next = next->next;
        if (next->next) next->next->prev = block;
        free_list_remove(next);
    }
}

// Public API
bool allocator_init(size_t pool_size) {
    // Align pool size to ALIGNMENT
    pool_size = ALIGN_UP(pool_size);
    g_pool = (uint8_t*)malloc(pool_size);
    if (!g_pool) return false;
    g_pool_size = pool_size;

    // Create a single large free block covering the whole pool
    g_head = (BlockHeader*)g_pool;
    g_head->size = pool_size - sizeof(BlockHeader);
    g_head->free = true;
    g_head->next = g_head->prev = NULL;
    g_head->next_free = g_head->prev_free = NULL;

    g_free_head = g_head;
    return true;
}

void* allocator_alloc(size_t size) {
    if (!g_pool) return NULL; // Not initialized
    if (size == 0) return NULL;

    size = ALIGN_UP(size);
    // First‑fit search in free list
    BlockHeader* cur = g_free_head;
    while (cur) {
        if (cur->size >= size) {
            // Found suitable block
            cur->free = false;
            free_list_remove(cur);
            split_block(cur, size);
            // Return pointer just after header
            return (void*)((uint8_t*)cur + sizeof(BlockHeader));
        }
        cur = cur->next_free;
    }
    // Out of memory
    return NULL;
}

void allocator_free(void* ptr) {
    if (!ptr) return;
    // Retrieve header
    BlockHeader* block = (BlockHeader*)((uint8_t*)ptr - sizeof(BlockHeader));
    assert(block->free == false); // Double free detection in debug builds
    block->free = true;
    free_list_insert(block);
    coalesce(block);
}

void allocator_destroy(void) {
    if (g_pool) {
        free(g_pool);
        g_pool = NULL;
        g_pool_size = 0;
        g_head = NULL;
        g_free_head = NULL;
    }
}
