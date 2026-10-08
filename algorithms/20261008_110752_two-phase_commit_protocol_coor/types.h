#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    PARTICIPANT_INIT,
    PARTICIPANT_PREPARED,
    PARTICIPANT_COMMITTED,
    PARTICIPANT_ABORTED
} participant_state_t;

typedef enum {
    COORDINATOR_INIT,
    COORDINATOR_WAITING,
    COORDINATOR_COMMIT,
    COORDINATOR_ABORT
} coordinator_state_t;

typedef enum {
    MSG_PREPARE,
    MSG_COMMIT,
    MSG_ABORT
} message_t;
