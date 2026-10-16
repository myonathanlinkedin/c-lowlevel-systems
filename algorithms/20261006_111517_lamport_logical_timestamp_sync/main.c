#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include <stdio.h>
#include <assert.h>
#include "types.h"

int main(void) {
    /* Test 1: Initialization */
    LamportClock clkA;
    lamport_init(&clkA, 0);
    assert(lamport_get(&clkA) == 0);

    /* Test 2: Internal event tick */
    lamport_tick(&clkA);
    assert(lamport_get(&clkA) == 1);

    /* Test 3: Send operation */
    uint64_t send_ts = lamport_send(&clkA);
    assert(send_ts == 2);
    assert(lamport_get(&clkA) == 2);

    /* Test 4: Receive operation with lower timestamp */
    LamportClock clkB;
    lamport_init(&clkB, 0);
    uint64_t recv_ts_low = lamport_receive(&clkB, send_ts); // recv_ts_low = 2
    assert(recv_ts_low == 3);
    assert(lamport_get(&clkB) == 3);

    /* Test 5: Receive operation with higher timestamp */
    lamport_init(&clkA, 10);
    uint64_t recv_ts_high = lamport_receive(&clkA, 20);
    assert(recv_ts_high == 21);
    assert(lamport_get(&clkA) == 21);

    /* Test 6: Multiple sequential events */
    lamport_init(&clkA, 5);
    lamport_tick(&clkA);               // 6
    uint64_t ts1 = lamport_send(&clkA); // 7
    assert(ts1 == 7);
    uint64_t ts2 = lamport_receive(&clkA, 7); // max(7,7)+1 = 8
    assert(ts2 == 8);
    lamport_tick(&clkA);               // 9
    assert(lamport_get(&clkA) == 9);

    /* Test 7: Edge case near UINT64_MAX (no overflow expected in normal use) */
    lamport_init(&clkA, UINT64_MAX - 2);
    lamport_tick(&clkA);               // UINT64_MAX -1
    uint64_t ts_send = lamport_send(&clkA); // UINT64_MAX
    assert(ts_send == UINT64_MAX);
    /* Receiving a timestamp equal to UINT64_MAX should wrap to 0 in unsigned arithmetic,
       but logical clocks are expected not to overflow in realistic scenarios.
       Here we verify the arithmetic behavior. */
    uint64_t ts_recv = lamport_receive(&clkA, UINT64_MAX);
    assert(ts_recv == 0); // overflow wraps around
    assert(lamport_get(&clkA) == 0);

    printf("All Lamport clock unit tests passed.\n");
    return 0;
}
