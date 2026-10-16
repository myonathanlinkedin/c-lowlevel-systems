#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Lamport logical clock structure */
typedef struct {
    uint64_t timestamp;
} LamportClock;

/* Initialize a Lamport clock with a given starting timestamp */
static inline void lamport_init(LamportClock *clk, uint64_t start) {
    clk->timestamp = start;
}

/* Increment the clock for an internal event */
static inline void lamport_tick(LamportClock *clk) {
    clk->timestamp += 1ULL;
}

/* Perform a send operation: increment clock and obtain timestamp to attach to a message */
static inline uint64_t lamport_send(LamportClock *clk) {
    lamport_tick(clk);
    return clk->timestamp;
}

/* Perform a receive operation: update clock based on received timestamp and return new value */
static inline uint64_t lamport_receive(LamportClock *clk, uint64_t recv_ts) {
    uint64_t max_ts = (clk->timestamp > recv_ts) ? clk->timestamp : recv_ts;
    clk->timestamp = max_ts + 1ULL;
    return clk->timestamp;
}

/* Retrieve the current timestamp */
static inline uint64_t lamport_get(const LamportClock *clk) {
    return clk->timestamp;
}
