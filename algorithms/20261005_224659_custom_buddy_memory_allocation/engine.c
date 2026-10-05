#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

BuddyAllocator g_allocator = {0};

static size_t order_to_size(uint8_t order) {
    return ((size_t)1) << order;
}

static uint8_t size_to_order(size_t size) {
    uint8_t order = BUDDY_MIN_ORDER;
    size_t block = order_to_size(order);
    while (block < size) {
        ++order;
        block <<= 1;
    }
    return order;
}

static BlockHeader* block_from_ptr(void* ptr) {
    return (BlockHeader*)((uint8_t*)ptr - sizeof(BlockHeader));
}

static void* ptr_from_block(BlockHeader* blk) {
    return (void*)((uint8_t*)blk + sizeof(BlockHeader));
}

static void insert_free(BlockHeader* blk) {
    uint8_t order = blk->order;
    blk->is_free = true;
    blk->next = g_allocator.free_lists[order];
    blk->prev = NULL;
    if (blk->next) {
        blk->next->prev = blk;
    }
    g_allocator.free_lists[order] = blk;
}

static void remove_free(BlockHeader* blk) {
    uint8_t order = blk->order;
    if (blk->prev) {
        blk->prev->next = blk->next;
    } else {
        g_allocator.free_lists[order] = blk->next;
    }
    if (blk->next) {
        blk->next->prev = blk->prev;
    }
    blk->next = blk->prev = NULL;
    blk->is_free = false;
}

static BlockHeader* split(BlockHeader* blk, uint8_t target_order) {
    while (blk->order > target_order) {
        uint8_t new_order = blk->order - 1;
        size_t block_sz = order_to_size(new_order);
        BlockHeader* buddy = (BlockHeader*)((uint8_t*)blk + block_sz);
        buddy->order = new_order;
        buddy->is_free = true;
        insert_free(buddy);
        blk->order = new_order;
    }
    return blk;
}

static BlockHeader* find_block(uint8_t order) {
    for (uint8_t o = order; o <= BUDDY_MAX_ORDER; ++o) {
        if (g_allocator.free_lists[o]) {
            BlockHeader* blk = g_allocator.free_lists[o];
            remove_free(blk);
            return split(blk, order);
        }
    }
    return NULL;
}

static BlockHeader* get_buddy(BlockHeader* blk) {
    size_t block_sz = order_to_size(blk->order);
    uintptr_t offset = (uintptr_t)((uint8_t*)blk - (uint8_t*)g_allocator.memory);
    uintptr_t buddy_offset = offset ^ block_sz;
    return (BlockHeader*)((uint8_t*)g_allocator.memory + buddy_offset);
}

static void try_coalesce(BlockHeader* blk) {
    while (blk->order < BUDDY_MAX_ORDER) {
        BlockHeader* buddy = get_buddy(blk);
        if (!buddy->is_free || buddy->order != blk->order) {
            break;
        }
        remove_free(buddy);
        if (buddy < blk) {
            blk = buddy;
        }
        blk->order += 1;
    }
    insert_free(blk);
}

bool buddy_init(size_t size) {
    size_t total = 1;
    uint8_t order = 0;
    while (total < size) {
        total <<= 1;
        ++order;
    }
    if (order < BUDDY_MIN_ORDER) {
        order = BUDDY_MIN_ORDER;
        total = order_to_size(order);
    }
    if (order > BUDDY_MAX_ORDER) {
        return false;
    }
    g_allocator.memory = malloc(total);
    if (!g_allocator.memory) {
        return false;
    }
    g_allocator.total_size = total;
    memset(g_allocator.free_lists, 0, sizeof(g_allocator.free_lists));
    BlockHeader* initial = (BlockHeader*)g_allocator.memory;
    initial->order = order;
    initial->is_free = true;
    initial->next = initial->prev = NULL;
    g_allocator.free_lists[order] = initial;
    return true;
}

void* buddy_alloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    size_t needed = size + sizeof(BlockHeader);
    uint8_t order = size_to_order(needed);
    if (order > BUDDY_MAX_ORDER) {
        return NULL;
    }
    BlockHeader* blk = find_block(order);
    if (!blk) {
        return NULL;
    }
    blk->is_free = false;
    blk->next = blk->prev = NULL;
    return ptr_from_block(blk);
}

void buddy_free(void* ptr) {
    if (!ptr) {
        return;
    }
    BlockHeader* blk = block_from_ptr(ptr);
    assert(!blk->is_free); // Detect double free
    blk->is_free = true;
    try_coalesce(blk);
}

void buddy_dump(void) {
    printf("Buddy Allocator Dump (total %zu bytes):\n", g_allocator.total_size);
    for (uint8_t o = BUDDY_MIN_ORDER; o <= BUDDY_MAX_ORDER; ++o) {
        printf(" Order %u: ", o);
        BlockHeader* cur = g_allocator.free_lists[o];
        while (cur) {
            printf("[addr=%p size=%zu] ", (void*)cur, order_to_size(cur->order));
            cur = cur->next;
        }
        printf("\n");
    }
}
