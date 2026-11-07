#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"

void lamport_init(LamportClock* clk, uint64_t start) {
    if (clk) {
        clk->time = start;
    }
}

void lamport_tick(LamportClock* clk) {
    if (clk) {
        /* Increment is defined modulo 2^64; overflow is well‑defined for unsigned types. */
        clk->time += 1ULL;
    }
}

void lamport_update(LamportClock* clk, uint64_t received) {
    if (clk) {
        /* The rule: C = max(C, received) + 1 */
        uint64_t max = (clk->time > received) ? clk->time : received;
        clk->time = max + 1ULL;
    }
}

uint64_t lamport_get(const LamportClock* clk) {
    return clk ? clk->time : 0ULL;
}

bool lamport_before(uint64_t a, uint64_t b) {
    return a < b;
}

Event make_event(int pid, const LamportClock* clk, const char* description) {
    Event ev;
    ev.pid = pid;
    ev.timestamp = lamport_get(clk);
    ev.description = description;
    return ev;
}

int lamport_event_compare(const Event* e1, const Event* e2) {
    if (!e1 || !e2) return 0;
    if (e1->timestamp < e2->timestamp) return -1;
    if (e1->timestamp > e2->timestamp) return 1;
    /* Timestamps equal – break ties by pid to obtain a total order (common in practice). */
    if (e1->pid < e2->pid) return -1;
    if (e1->pid > e2->pid) return 1;
    return 0;
}
