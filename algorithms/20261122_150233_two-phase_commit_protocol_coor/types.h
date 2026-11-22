#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    COORD_INIT,
    COORD_PREPARE,
    COORD_COMMIT,
    COORD_ABORT,
    COORD_DONE
} coordinator_state_t;

typedef enum {
    PART_INIT,
    PART_READY,
    PART_COMMITTED,
    PART_ABORTED
} participant_state_t;

typedef struct participant participant_t;
typedef struct coordinator coordinator_t;

/* Participant interface */
typedef struct {
    participant_t *self;
    bool (*receive_prepare)(participant_t *self);
    void (*receive_commit)(participant_t *self);
    void (*receive_abort)(participant_t *self);
} participant_vtable_t;

/* Participant object */
struct participant {
    uint32_t id;
    participant_state_t state;
    participant_vtable_t *vtable;
    /* For testing: predetermined vote (true = yes, false = no) */
    bool vote_yes;
};

/* Coordinator object */
struct coordinator {
    coordinator_state_t state;
    participant_t **participants;
    size_t participant_cnt;
    size_t votes_yes;
    size_t votes_no;
};

/* Result of a vote collection */
typedef enum {
    VOTE_PENDING,
    VOTE_COMMIT,
    VOTE_ABORT
} vote_result_t;
