#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "types.h"
#include "core.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void test_basic_read_write(void) {
    VirtualMemoryManager vm;
    vm_init(&vm);

    size_t vaddr = vm_alloc(&vm, 2); // allocate 2 pages (8192 bytes)
    const char pattern[] = "Hello, Ember Unified Virtual Memory!";
    vm_write(&vm, vaddr, pattern, sizeof(pattern));

    char buffer[sizeof(pattern)];
    memset(buffer, 0, sizeof(buffer));
    vm_read(&vm, vaddr, buffer, sizeof(buffer));
    assert(memcmp(buffer, pattern, sizeof(pattern)) == 0);

    vm_cleanup(&vm);
    printf("test_basic_read_write passed.\n");
}

/* Fill physical memory, then allocate extra pages to force eviction. */
static void test_eviction_lru(void) {
    VirtualMemoryManager vm;
    vm_init(&vm);

    // Allocate enough virtual pages to exceed physical capacity.
    size_t pages_needed = MAX_PHYSICAL_PAGES + 10;
    size_t vaddr_base = vm_alloc(&vm, pages_needed);

    // Write distinct byte to each page to identify them later.
    for (size_t i = 0; i < pages_needed; ++i) {
        uint8_t value = (uint8_t)(i & 0xFF);
        vm_write(&vm, vaddr_base + i * PAGE_SIZE, &value, 1);
    }

    // Read back first few pages; they may have been evicted and reloaded.
    for (size_t i = 0; i < pages_needed; ++i) {
        uint8_t read_val = 0xFF;
        vm_read(&vm, vaddr_base + i * PAGE_SIZE, &read_val, 1);
        assert(read_val == (uint8_t)(i & 0xFF));
    }

    vm_cleanup(&vm);
    printf("test_eviction_lru passed.\n");
}

/* Verify that dirty pages are written back to swap on eviction. */
static void test_dirty_writeback(void) {
    VirtualMemoryManager vm;
    vm_init(&vm);

    // Allocate exactly physical capacity pages.
    size_t vaddr1 = vm_alloc(&vm, MAX_PHYSICAL_PAGES);
    // Write unique data to each page.
    for (size_t i = 0; i < MAX_PHYSICAL_PAGES; ++i) {
        uint8_t val = (uint8_t)(i + 1);
        vm_write(&vm, vaddr1 + i * PAGE_SIZE, &val, 1);
    }

    // Allocate one more page to trigger eviction of the LRU (page 0).
    size_t vaddr_extra = vm_alloc(&vm, 1);
    uint8_t dummy = 0xAA;
    vm_write(&vm, vaddr_extra, &dummy, 1);

    // Now read back page 0; it should have been evicted, written back, and reloaded.
    uint8_t read_back = 0;
    vm_read(&vm, vaddr1, &read_back, 1);
    assert(read_back == 1); // original value

    vm_cleanup(&vm);
    printf("test_dirty_writeback passed.\n");
}

int main(void) {
    test_basic_read_write();
    test_eviction_lru();
    test_dirty_writeback();
    printf("All Ember VM tests passed.\n");
    return 0;
}
