#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint64_t time;
} LamportClock;

typedef struct {
    int pid;                 // Identifier of the originating process
    uint64_t timestamp;      // Lamport timestamp of the event
    const char* description; // Human‑readable label (non‑owned string)
} Event;
