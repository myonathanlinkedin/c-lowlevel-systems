#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#pragma once
#include "types.h"

/* Initialise a Lamport clock with a given start value. */
void lamport_init(LamportClock* clk, uint64_t start);

/* Increment the local clock (happens before a local event). */
void lamport_tick(LamportClock* clk);

/* Incorporate a received timestamp (happens after a message receipt). */
void lamport_update(LamportClock* clk, uint64_t received);

/* Retrieve the current timestamp value. */
uint64_t lamport_get(const LamportClock* clk);

/* Return true if a < b in Lamport order. */
bool lamport_before(uint64_t a, uint64_t b);

/* Return true if a and b are concurrent – for pure Lamport clocks this is always false. */
static inline bool lamport_concurrent(uint64_t a, uint64_t b) { (void)a; (void)b; return false; }

/* Create an Event record from a clock snapshot. */
Event make_event(int pid, const LamportClock* clk, const char* description);

/* Compare two events according to Lamport ordering.
   Returns -1 if e1 precedes e2, 0 if equal timestamps, 1 if e1 follows e2. */
int lamport_event_compare(const Event* e1, const Event* e2);
