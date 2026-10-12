#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include "core.h"
#include <string.h>
#include <assert.h>

static size_t vm_find_free_phys_page(VirtualMemoryManager *vm);
static void vm_evict_lru_page(VirtualMemoryManager *vm);
static void vm_handle_page_fault(VirtualMemoryManager *vm, size_t vpage);

void vm_init(VirtualMemoryManager *vm) {
    assert(vm != NULL);
    memset(vm, 0, sizeof(*vm));
    vm->next_vpage = 0;
    vm->tick = 0;
    // All entries are already zeroed (invalid, not dirty, etc.)
}

size_t vm_alloc(VirtualMemoryManager *vm, size_t num_pages) {
    assert(vm != NULL);
    assert(num_pages > 0);
    assert(vm->next_vpage + num_pages <= MAX_VIRTUAL_PAGES);
    size_t base = vm->next_vpage;
    for (size_t i = 0; i < num_pages; ++i) {
        PageTableEntry *pte = &vm->page_table[base + i];
        pte->valid = false;   // Not yet mapped
        pte->dirty = false;
        pte->phys_page = 0;
        pte->last_used = 0;
        // Zero swap storage for deterministic behavior
        memset(vm->swap_storage[base + i], 0, PAGE_SIZE);
    }
    vm->next_vpage += num_pages;
    return base * PAGE_SIZE; // Return byte address of first page
}

static void vm_update_timestamp(VirtualMemoryManager *vm, size_t vpage) {
    vm->tick++;
    vm->page_table[vpage].last_used = vm->tick;
}

/* Ensure the virtual page is resident; if not, bring it in (handle fault). */
static void vm_ensure_resident(VirtualMemoryManager *vm, size_t vpage) {
    assert(vpage < MAX_VIRTUAL_PAGES);
    PageTableEntry *pte = &vm->page_table[vpage];
    if (!pte->valid) {
        vm_handle_page_fault(vm, vpage);
    }
    vm_update_timestamp(vm, vpage);
}

/* Page fault handling: allocate a physical page, possibly evicting another. */
static void vm_handle_page_fault(VirtualMemoryManager *vm, size_t vpage) {
    size_t phys = vm_find_free_phys_page(vm);
    if (phys == (size_t)-1) {
        vm_evict_lru_page(vm);
        phys = vm_find_free_phys_page(vm);
        assert(phys != (size_t)-1); // After eviction there must be free page
    }
    // Load data from swap (or zero if never written)
    memcpy(vm->phys_mem[phys].data, vm->swap_storage[vpage], PAGE_SIZE);
    vm->phys_mem[phys].occupied = true;

    PageTableEntry *pte = &vm->page_table[vpage];
    pte->valid = true;
    pte->phys_page = phys;
    // dirty flag remains false until a write occurs
}

/* Find a free physical page; return (size_t)-1 if none. */
static size_t vm_find_free_phys_page(VirtualMemoryManager *vm) {
    for (size_t i = 0; i < MAX_PHYSICAL_PAGES; ++i) {
        if (!vm->phys_mem[i].occupied) {
            return i;
        }
    }
    return (size_t)-1;
}

/* Evict the least-recently-used resident page. */
static void vm_evict_lru_page(VirtualMemoryManager *vm) {
    uint64_t oldest = UINT64_MAX;
    size_t victim_vpage = (size_t)-1;
    size_t victim_ppage = (size_t)-1;

    for (size_t vp = 0; vp < vm->next_vpage; ++vp) {
        PageTableEntry *pte = &vm->page_table[vp];
        if (pte->valid) {
            if (pte->last_used < oldest) {
                oldest = pte->last_used;
                victim_vpage = vp;
                victim_ppage = pte->phys_page;
            }
        }
    }
    assert(victim_vpage != (size_t)-1); // There must be at least one resident page

    // Write back if dirty
    if (vm->page_table[victim_vpage].dirty) {
        memcpy(vm->swap_storage[victim_vpage],
               vm->phys_mem[victim_ppage].data,
               PAGE_SIZE);
        vm->page_table[victim_vpage].dirty = false;
    }

    // Mark physical page free
    vm->phys_mem[victim_ppage].occupied = false;
    // Invalidate virtual mapping
    vm->page_table[victim_vpage].valid = false;
}

/* Public write: may span multiple pages. */
void vm_write(VirtualMemoryManager *vm, size_t vaddr, const void *src, size_t len) {
    assert(vm != NULL);
    assert(src != NULL);
    size_t remaining = len;
    const uint8_t *src_ptr = (const uint8_t *)src;
    size_t cur_addr = vaddr;

    while (remaining > 0) {
        size_t vpage = cur_addr / PAGE_SIZE;
        size_t page_offset = cur_addr % PAGE_SIZE;
        size_t to_copy = PAGE_SIZE - page_offset;
        if (to_copy > remaining) to_copy = remaining;

        vm_ensure_resident(vm, vpage);
        PageTableEntry *pte = &vm->page_table[vpage];
        size_t phys = pte->phys_page;
        memcpy(&vm->phys_mem[phys].data[page_offset], src_ptr, to_copy);
        pte->dirty = true; // Mark dirty after any write

        src_ptr += to_copy;
        cur_addr += to_copy;
        remaining -= to_copy;
    }
}

/* Public read: may span multiple pages. */
void vm_read(VirtualMemoryManager *vm, size_t vaddr, void *dst, size_t len) {
    assert(vm != NULL);
    assert(dst != NULL);
    size_t remaining = len;
    uint8_t *dst_ptr = (uint8_t *)dst;
    size_t cur_addr = vaddr;

    while (remaining > 0) {
        size_t vpage = cur_addr / PAGE_SIZE;
        size_t page_offset = cur_addr % PAGE_SIZE;
        size_t to_copy = PAGE_SIZE - page_offset;
        if (to_copy > remaining) to_copy = remaining;

        vm_ensure_resident(vm, vpage);
        PageTableEntry *pte = &vm->page_table[vpage];
        size_t phys = pte->phys_page;
        memcpy(dst_ptr, &vm->phys_mem[phys].data[page_offset], to_copy);

        dst_ptr += to_copy;
        cur_addr += to_copy;
        remaining -= to_copy;
    }
}

/* Cleanup: free all resources (no dynamic allocation here, but reset state). */
void vm_cleanup(VirtualMemoryManager *vm) {
    assert(vm != NULL);
    memset(vm, 0, sizeof(*vm));
}
