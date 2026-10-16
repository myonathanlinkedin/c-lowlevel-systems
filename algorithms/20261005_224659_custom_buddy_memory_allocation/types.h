#include <stdlib.h>

#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define BUDDY_MAX_ORDER 20   // Supports up to 1 << 20 bytes (1 MiB)
#define BUDDY_MIN_ORDER 4    // Minimum block size 16 bytes

typedef struct BlockHeader {
    uint8_t order;               // Exponent of block size (2^order)
    bool is_free;                // Free flag
    struct BlockHeader* next;    // Next block in free list
    struct BlockHeader* prev;    // Previous block in free list
} BlockHeader;

typedef struct BuddyAllocator {
    void* memory;                                 // Base address of pool
    size_t total_size;                            // Total pool size
    BlockHeader* free_lists[BUDDY_MAX_ORDER + 1]; // Array of free lists per order
} BuddyAllocator;

extern BuddyAllocator g_allocator;

bool buddy_init(size_t size);
void* buddy_alloc(size_t size);
void buddy_free(void* ptr);
void buddy_dump(void);

#endif // TYPES_H
