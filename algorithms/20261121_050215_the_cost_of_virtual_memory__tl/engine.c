#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/*--------------------------------------------------------------
 * Analytical formulas implementation.
 *--------------------------------------------------------------*/
double compute_eat(double tlb_access, double mem_access, double hit_rate)
{
    /* Guard against invalid probabilities. */
    if (hit_rate < 0.0) hit_rate = 0.0;
    if (hit_rate > 1.0) hit_rate = 1.0;

    /* EAT = hit_rate * (TLB + MEM) + (1 - hit_rate) * (TLB + 2*MEM) */
    double hit_time  = tlb_access + mem_access;
    double miss_time = tlb_access + 2.0 * mem_access;
    return hit_rate * hit_time + (1.0 - hit_rate) * miss_time;
}

double compute_eat_with_page_fault(double tlb_access,
                                   double mem_access,
                                   double hit_rate,
                                   double page_fault_rate,
                                   double page_fault_service_time)
{
    if (page_fault_rate < 0.0) page_fault_rate = 0.0;
    if (page_fault_rate > 1.0) page_fault_rate = 1.0;

    double eat_no_fault = compute_eat(tlb_access, mem_access, hit_rate);
    return (1.0 - page_fault_rate) * eat_no_fault
         + page_fault_rate * page_fault_service_time;
}

/*--------------------------------------------------------------
 * TLB simulation implementation.
 *--------------------------------------------------------------*/
static TLBEntry *tlb_find_entry(TLB *tlb, uint32_t vpn)
{
    for (size_t i = 0; i < tlb->capacity; ++i) {
        TLBEntry *e = &tlb->entries[i];
        if (e->valid && e->vpn == vpn) {
            return e;
        }
    }
    return NULL;
}

/* Find the LRU entry (valid or invalid) for replacement. */
static TLBEntry *tlb_find_victim(TLB *tlb)
{
    TLBEntry *victim = NULL;
    uint64_t oldest = UINT64_MAX;

    for (size_t i = 0; i < tlb->capacity; ++i) {
        TLBEntry *e = &tlb->entries[i];
        if (!e->valid) {
            return e; /* free slot found */
        }
        if (e->lru_counter < oldest) {
            oldest = e->lru_counter;
            victim = e;
        }
    }
    return victim; /* must be non‑NULL when capacity > 0 */
}

TLB *tlb_create(size_t capacity)
{
    TLB *tlb = (TLB *)malloc(sizeof(TLB));
    if (!tlb) {
        return NULL;
    }
    tlb->capacity = capacity;
    tlb->count = 0;
    tlb->access_seq = 0;
    if (capacity == 0) {
        tlb->entries = NULL;
        return tlb;
    }
    tlb->entries = (TLBEntry *)calloc(capacity, sizeof(TLBEntry));
    if (!tlb->entries) {
        free(tlb);
        return NULL;
    }
    for (size_t i = 0; i < capacity; ++i) {
        tlb->entries[i].valid = false;
    }
    return tlb;
}

void tlb_destroy(TLB *tlb)
{
    if (!tlb) return;
    free(tlb->entries);
    free(tlb);
}

bool tlb_lookup(TLB *tlb, uint32_t vpn, uint32_t *pfn_out)
{
    if (!tlb || tlb->capacity == 0) {
        return false;
    }
    TLBEntry *e = tlb_find_entry(tlb, vpn);
    if (e) {
        /* Hit – update LRU counter */
        tlb->access_seq++;
        e->lru_counter = tlb->access_seq;
        if (pfn_out) {
            *pfn_out = e->pfn;
        }
        return true;
    }
    return false; /* Miss */
}

void tlb_insert(TLB *tlb, uint32_t vpn, uint32_t pfn)
{
    if (!tlb || tlb->capacity == 0) {
        return;
    }
    TLBEntry *e = tlb_find_entry(tlb, vpn);
    if (e) {
        /* Update existing entry */
        e->pfn = pfn;
        tlb->access_seq++;
        e->lru_counter = tlb->access_seq;
        return;
    }

    /* Need a new entry – possibly evict LRU */
    TLBEntry *victim = tlb_find_victim(tlb);
    assert(victim != NULL); /* capacity > 0 guarantees a victim */

    victim->vpn = vpn;
    victim->pfn = pfn;
    victim->valid = true;
    tlb->access_seq++;
    victim->lru_counter = tlb->access_seq;

    if (tlb->count < tlb->capacity) {
        tlb->count++;
    }
}
