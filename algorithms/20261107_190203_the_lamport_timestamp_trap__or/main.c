#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "core.h"

static void test_basic_operations(void) {
    LamportClock clk1, clk2;
    lamport_init(&clk1, 0);
    lamport_init(&clk2, 0);

    /* Process 1 performs two local events. */
    lamport_tick(&clk1);               // C1 = 1
    Event e1 = make_event(1, &clk1, "P1 event A");
    lamport_tick(&clk1);               // C1 = 2
    Event e2 = make_event(1, &clk1, "P1 event B");

    /* Process 2 performs one local event. */
    lamport_tick(&clk2);               // C2 = 1
    Event e3 = make_event(2, &clk2, "P2 event A");

    /* Verify ordering of timestamps. */
    assert(lamport_before(e1.timestamp, e2.timestamp));
    assert(lamport_before(e1.timestamp, e3.timestamp) == false);
    assert(lamport_before(e3.timestamp, e2.timestamp));

    /* Simulate a message from P1 (e2) to P2. */
    lamport_update(&clk2, e2.timestamp); // C2 = max(1,2)+1 = 3
    Event e4 = make_event(2, &clk2, "P2 after receiving P1 B");

    assert(e4.timestamp == 3);
    assert(lamport_before(e2.timestamp, e4.timestamp));
    assert(lamport_event_compare(&e2, &e4) == -1);
}

static void test_total_order_vs_causality(void) {
    /* Two independent processes generate events without communication.
       Their timestamps interleave, but there is no causal relation. */
    LamportClock a, b;
    lamport_init(&a, 0);
    lamport_init(&b, 0);

    lamport_tick(&a); // A1 = 1
    Event A1 = make_event(1, &a, "A1");
    lamport_tick(&b); // B1 = 1
    Event B1 = make_event(2, &b, "B1");
    lamport_tick(&a); // A2 = 2
    Event A2 = make_event(1, &a, "A2");
    lamport_tick(&b); // B2 = 2
    Event B2 = make_event(2, &b, "B2");

    /* Total order imposed by timestamps (and pid tie‑breaker). */
    int cmp_A1_B1 = lamport_event_compare(&A1, &B1);
    int cmp_B1_A2 = lamport_event_compare(&B1, &A2);
    int cmp_A2_B2 = lamport_event_compare(&A2, &B2);

    /* Because timestamps are equal for A1/B1 and A2/B2, pid decides order. */
    assert(cmp_A1_B1 == -1); // pid 1 < pid 2
    assert(cmp_B1_A2 == -1); // 1 < 2 (timestamp 1 < 2)
    assert(cmp_A2_B2 == -1); // pid 1 < pid 2

    /* However, there is no causal relationship between A1 and B1. */
    assert(lamport_concurrent(A1.timestamp, B1.timestamp) == false); // Lamport cannot express concurrency
    /* The test demonstrates the "trap": ordered numbers do not imply causality. */
}

static void test_overflow_behavior(void) {
    LamportClock clk;
    lamport_init(&clk, UINT64_MAX - 1);
    lamport_tick(&clk); // becomes UINT64_MAX
    assert(lamport_get(&clk) == UINT64_MAX);
    lamport_tick(&clk); // wraps to 0
    assert(lamport_get(&clk) == 0ULL);
    /* After wrap‑around, ordering is still defined by unsigned comparison. */
    assert(lamport_before(0ULL, 1ULL));
    assert(!lamport_before(UINT64_MAX, 0ULL));
}

int main(void) {
    test_basic_operations();
    test_total_order_vs_causality();
    test_overflow_behavior();

    printf("All Lamport timestamp tests passed.\n");
    return EXIT_SUCCESS;
}
