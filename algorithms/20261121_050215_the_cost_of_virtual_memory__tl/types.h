#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/*--------------------------------------------------------------
 * Data structures for a simple fully‑associative TLB with LRU.
 *--------------------------------------------------------------*/
typedef struct {
    uint32_t vpn;      /* virtual page number */
    uint32_t pfn;      /* physical frame number */
    bool     valid;    /* entry validity */
    uint64_t lru_counter; /* monotonically increasing counter for LRU */
} TLBEntry;

typedef struct {
    size_t    capacity;   /* maximum number of entries */
    size_t    count;      /* current number of valid entries */
    uint64_t  access_seq; /* global counter to implement LRU */
    TLBEntry *entries;    /* dynamically allocated array of entries */
} TLB;

/*--------------------------------------------------------------
 * Public API – analytical formulas.
 *--------------------------------------------------------------*/
/* Compute effective access time (EAT) without page faults.
 * tlb_access   : time to access the TLB (ns, cycles, etc.)
 * mem_access   : time to access main memory
 * hit_rate     : probability that a virtual address hits in the TLB (0.0 … 1.0)
 * Returns the average access time.
 */
double compute_eat(double tlb_access, double mem_access, double hit_rate);

/* Compute effective access time (EAT) including page‑fault cost.
 * page_fault_rate          : probability that a reference causes a page fault (0.0 … 1.0)
 * page_fault_service_time  : time to service a page fault (including disk I/O)
 * Returns the average access time.
 */
double compute_eat_with_page_fault(double tlb_access,
                                   double mem_access,
                                   double hit_rate,
                                   double page_fault_rate,
                                   double page_fault_service_time);

/*--------------------------------------------------------------
 * Public API – TLB simulation.
 *--------------------------------------------------------------*/
TLB *tlb_create(size_t capacity);
void  tlb_destroy(TLB *tlb);

/* Lookup a VPN in the TLB.
 * Returns true and stores the PFN in *pfn_out if hit; otherwise false.
 * The function updates LRU information on a hit.
 */
bool tlb_lookup(TLB *tlb, uint32_t vpn, uint32_t *pfn_out);

/* Insert or update a VPN→PFN mapping.
 * If the TLB is full, the least‑recently‑used entry is evicted.
 */
void tlb_insert(TLB *tlb, uint32_t vpn, uint32_t pfn);
