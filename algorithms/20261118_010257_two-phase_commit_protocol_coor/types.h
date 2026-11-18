#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/* ---------- Enumerations ---------- */
typedef enum {
    PARTICIPANT_INIT,
    PARTICIPANT_PREPARED,
    PARTICIPANT_COMMITTED,
    PARTICIPANT_ABORTED
} participant_state_t;

typedef enum {
    COORDINATOR_IDLE,
    COORDINATOR_PREPARING,
    COORDINATOR_COMMITTING,
    COORDINATOR_ABORTING
} coordinator_state_t;

/* ---------- Structures ---------- */
#define MAX_PARTICIPANTS 16

typedef struct participant {
    /* Identifier for debugging / testing */
    const char *name;
    /* Pre‑programmed vote for the current transaction */
    bool vote_commit;               /* true → vote COMMIT, false → vote ABORT */
    participant_state_t state;
} participant_t;

typedef struct coordinator {
    participant_t *participants[MAX_PARTICIPANTS];
    size_t participant_count;
    coordinator_state_t state;
} coordinator_t;

/* ---------- Public API ---------- */
#ifdef __cplusplus
extern "C" {
#endif

/* Coordinator lifecycle */
void coordinator_init(coordinator_t *coord);
bool coordinator_add_participant(coordinator_t *coord, participant_t *p);

/* Transaction execution */
void coordinator_start_transaction(coordinator_t *coord);

/* Participant helpers (used by the coordinator) */
static inline participant_state_t participant_get_state(const participant_t *p) {
    return p->state;
}

/* Test helpers */
void participant_reset(participant_t *p);
void participant_set_vote(participant_t *p, bool commit);

/* Internal functions – not part of the public API but needed for unit tests */
void coordinator_process_prepare(coordinator_t *coord);
void coordinator_process_decision(coordinator_t *coord, bool all_commit);

#ifdef __cplusplus
}
#endif
