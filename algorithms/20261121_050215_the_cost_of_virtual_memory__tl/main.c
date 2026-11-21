#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

/* Helper to compare floating‑point numbers within a tolerance. */
static bool approx_equal(double a, double b, double eps)
{
    return fabs(a - b) <= eps;
}

/* Unit tests for analytical formulas. */
static void test_compute_eat(void)
{
    /* Example from textbook:
     * tlb_access = 1 ns, mem_access = 100 ns, hit_rate = 0.8
     * Expected EAT = 0.8*(1+100) + 0.2*(1+200) = 0.8*101 + 0.2*201 = 80.8 + 40.2 = 121 ns
     */
    double eat = compute_eat(1.0, 100.0, 0.8);
    assert(approx_equal(eat, 121.0, 1e-9));

    /* Edge cases */
    assert(approx_equal(compute_eat(5.0, 50.0, 0.0), 5.0 + 2.0 * 50.0, 1e-9)); /* always miss */
    assert(approx_equal(compute_eat(5.0, 50.0, 1.0), 5.0 + 50.0, 1e-9));      /* always hit */
}

static void test_compute_eat_with_page_fault(void)
{
    /* No page faults – should equal compute_eat */
    double no_fault = compute_eat_with_page_fault(1.0, 100.0, 0.9, 0.0, 1e6);
    assert(approx_equal(no_fault, compute_eat(1.0, 100.0, 0.9), 1e-9));

    /* 100% page fault – result is the service time */
    double pf = compute_eat_with_page_fault(1.0, 100.0, 0.9, 1.0, 1e6);
    assert(approx_equal(pf, 1e6, 1e-9));

    /* Mixed case */
    double mixed = compute_eat_with_page_fault(2.0, 80.0, 0.7, 0.01, 1e5);
    double eat_no_fault = compute_eat(2.0, 80.0, 0.7);
    double expected = 0.99 * eat_no_fault + 0.01 * 1e5;
    assert(approx_equal(mixed, expected, 1e-6));
}

/* Unit tests for the TLB simulation. */
static void test_tlb_basic(void)
{
    TLB *tlb = tlb_create(2);
    assert(tlb != NULL);
    uint32_t pfn;

    /* Miss on empty TLB */
    assert(!tlb_lookup(tlb, 0xAAA, &pfn));

    /* Insert entry and hit */
    tlb_insert(tlb, 0xAAA, 0x111);
    assert(tlb_lookup(tlb, 0xAAA, &pfn));
    assert(pfn == 0x111);

    /* Insert second entry */
    tlb_insert(tlb, 0xBBB, 0x222);
    assert(tlb_lookup(tlb, 0xBBB, &pfn));
    assert(pfn == 0x222);

    /* Insert third entry – should evict LRU (0xAAA) */
    tlb_insert(tlb, 0xCCC, 0x333);
    assert(!tlb_lookup(tlb, 0xAAA, &pfn)); /* evicted */
    assert(tlb_lookup(tlb, 0xBBB, &pfn));
    assert(pfn == 0x222);
    assert(tlb_lookup(tlb, 0xCCC, &pfn));
    assert(pfn == 0x333);

    tlb_destroy(tlb);
}

/* Edge‑case tests: zero‑capacity TLB and repeated updates. */
static void test_tlb_edge_cases(void)
{
    TLB *zero = tlb_create(0);
    assert(zero != NULL);
    uint32_t pfn;
    assert(!tlb_lookup(zero, 0x1, &pfn));
    tlb_insert(zero, 0x1, 0x2); /* should be a no‑op, not crash */
    tlb_destroy(zero);

    /* Re‑inserting same VPN updates PFN and LRU */
    TLB *tlb = tlb_create(1);
    tlb_insert(tlb, 0x100, 0x200);
    assert(tlb_lookup(tlb, 0x100, &pfn));
    assert(pfn == 0x200);
    tlb_insert(tlb, 0x100, 0x300); /* update */
    assert(tlb_lookup(tlb, 0x100, &pfn));
    assert(pfn == 0x300);
    tlb_destroy(tlb);
}

/* Demonstration driver – prints a few example calculations. */
static void demo(void)
{
    double tlb_time = 1.0;   /* ns */
    double mem_time = 100.0; /* ns */
    double hit = 0.95;
    double pf_rate = 0.0001;
    double pf_service = 1e6; /* ns (≈1 ms) */

    double eat = compute_eat(tlb_time, mem_time, hit);
    double eat_pf = compute_eat_with_page_fault(tlb_time, mem_time, hit,
                                                pf_rate, pf_service);

    printf("EAT without page faults: %.2f ns\n", eat);
    printf("EAT with page‑fault rate %.5f: %.2f ns\n", pf_rate, eat_pf);
}

int main(void)
{
    test_compute_eat();
    test_compute_eat_with_page_fault();
    test_tlb_basic();
    test_tlb_edge_cases();

    demo();

    printf("All tests passed.\n");
    return 0;
}
