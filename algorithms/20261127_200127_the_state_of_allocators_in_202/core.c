#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <assert.h>
#include <stdio.h>

static uint8_t*      g_pool       = NULL;   /* start of the memory pool */
static size_t        g_pool_size  = 0;      /* size of the memory pool */
static block_header_t* g_free_list = NULL;  /* singly‑linked free list */

/* Forward declarations */
static void insert_free_block(block_header_t* block);
static void coalesce_free_list(void);

/* Initialise the allocator with a raw memory buffer. */
void allocator_init(void* pool, size_t size)
{
    assert(pool != NULL);
    assert(size > sizeof(block_header_t));

    g_pool      = (uint8_t*)pool;
    g_pool_size = size;

    /* Create a single free block that spans the whole pool. */
    block_header_t* initial = (block_header_t*)g_pool;
    initial->size = size;
    initial->free = true;
    initial->next = NULL;
    g_free_list = initial;
}

/* Allocate memory of at least `size` bytes, respecting ALIGNMENT. */
void* allocator_alloc(size_t size)
{
    if (size == 0) {
        return NULL;
    }

    size_t total = ALIGN_UP(size) + sizeof(block_header_t);
    block_header_t* prev = NULL;
    block_header_t* cur  = g_free_list;

    while (cur != NULL) {
        if (cur->free && cur->size >= total) {
            size_t remaining = cur->size - total;

            if (remaining >= sizeof(block_header_t) + ALIGNMENT) {
                /* Split the block. */
                block_header_t* next = (block_header_t*)((uint8_t*)cur + total);
                next->size = remaining;
                next->free = true;
                next->next = cur->next;

                cur->size = total;
                cur->free = false;
                cur->next = NULL;

                if (prev != NULL) {
                    prev->next = next;
                } else {
                    g_free_list = next;
                }
            } else {
                /* Use the whole block. */
                cur->free = false;
                if (prev != NULL) {
                    prev->next = cur->next;
                } else {
                    g_free_list = cur->next;
                }
                cur->next = NULL;
            }

            return (void*)((uint8_t*)cur + sizeof(block_header_t));
        }

        prev = cur;
        cur  = cur->next;
    }

    /* No suitable block found. */
    return NULL;
}

/* Return a block to the allocator. */
void allocator_free(void* ptr)
{
    if (ptr == NULL) {
        return;
    }

    block_header_t* header = (block_header_t*)((uint8_t*)ptr - sizeof(block_header_t));
    assert(header->free == false);   /* Detect double free. */

    header->free = true;
    insert_free_block(header);
    coalesce_free_list();
}

/* Insert a free block into the free list, keeping address order. */
static void insert_free_block(block_header_t* block)
{
    if (g_free_list == NULL || block < g_free_list) {
        block->next = g_free_list;
        g_free_list = block;
        return;
    }

    block_header_t* cur = g_free_list;
    while (cur->next != NULL && cur->next < block) {
        cur = cur->next;
    }

    block->next = cur->next;
    cur->next   = block;
}

/* Merge adjacent free blocks to avoid fragmentation. */
static void coalesce_free_list(void)
{
    block_header_t* cur = g_free_list;
    while (cur != NULL && cur->next != NULL) {
        uint8_t* cur_end = (uint8_t*)cur + cur->size;
        if (cur_end == (uint8_t*)cur->next) {
            /* Adjacent blocks – merge them. */
            cur->size += cur->next->size;
            cur->next  = cur->next->next;
        } else {
            cur = cur->next;
        }
    }
}

/* Diagnostic dump of the whole pool and the free list. */
void allocator_dump(void)
{
    printf("=== Allocator Dump ===\n");
    block_header_t* cur = (block_header_t*)g_pool;
    while ((uint8_t*)cur < g_pool + g_pool_size) {
        printf("Block %p: size=%zu, free=%s\n",
               (void*)cur,
               cur->size,
               cur->free ? "true" : "false");
        cur = (block_header_t*)((uint8_t*)cur + cur->size);
    }

    printf("--- Free List ---\n");
    cur = g_free_list;
    while (cur != NULL) {
        printf("Free %p size=%zu\n", (void*)cur, cur->size);
        cur = cur->next;
    }
    printf("=====================\n");
}
