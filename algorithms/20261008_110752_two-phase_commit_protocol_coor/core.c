#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <stdio.h>

/* ---------- Participant implementation ---------- */
struct participant {
    int id;
    bool vote_commit;               // true = vote commit, false = vote abort
    participant_state_t state;
};

participant_t *participant_create(int id) {
    participant_t *p = (participant_t *)malloc(sizeof(participant_t));
    if (!p) return NULL;
    p->id = id;
    p->vote_commit = true;          // default vote commit
    p->state = PARTICIPANT_INIT;
    return p;
}

void participant_destroy(participant_t *p) {
    free(p);
}

void participant_set_vote(participant_t *p, bool vote_commit) {
    p->vote_commit = vote_commit;
}

bool participant_get_vote(const participant_t *p) {
    return p->vote_commit;
}

participant_state_t participant_get_state(const participant_t *p) {
    return p->state;
}

/* Internal: handle a prepare request */
static void participant_handle_prepare(participant_t *p) {
    p->state = PARTICIPANT_PREPARED;
    /* In a real system, the vote would be sent back to coordinator.
       Here the coordinator reads participant_get_vote(). */
}

/* Internal: handle a commit request */
static void participant_handle_commit(participant_t *p) {
    p->state = PARTICIPANT_COMMITTED;
}

/* Internal: handle an abort request */
static void participant_handle_abort(participant_t *p) {
    p->state = PARTICIPANT_ABORTED;
}

void participant_receive_message(participant_t *p, message_t msg) {
    switch (msg) {
        case MSG_PREPARE:
            participant_handle_prepare(p);
            break;
        case MSG_COMMIT:
            participant_handle_commit(p);
            break;
        case MSG_ABORT:
            participant_handle_abort(p);
            break;
        default:
            /* ignore unknown messages */
            break;
    }
}

/* ---------- Coordinator implementation ---------- */
struct coordinator {
    participant_t **participants;
    size_t count;
    coordinator_state_t state;
};

coordinator_t *coordinator_create(participant_t **participants, size_t count) {
    coordinator_t *c = (coordinator_t *)malloc(sizeof(coordinator_t));
    if (!c) return NULL;
    c->participants = participants;
    c->count = count;
    c->state = COORDINATOR_INIT;
    return c;
}

void coordinator_destroy(coordinator_t *c) {
    free(c);
}

coordinator_state_t coordinator_get_state(const coordinator_t *c) {
    return c->state;
}

/* Send PREPARE to all participants */
void coordinator_start_transaction(coordinator_t *c) {
    if (c->state != COORDINATOR_INIT) return;
    c->state = COORDINATOR_WAITING;
    for (size_t i = 0; i < c->count; ++i) {
        participant_receive_message(c->participants[i], MSG_PREPARE);
    }
}

/* Collect votes and decide */
void coordinator_process_responses(coordinator_t *c) {
    if (c->state != COORDINATOR_WAITING) return;
    for (size_t i = 0; i < c->count; ++i) {
        if (!participant_get_vote(c->participants[i])) {
            c->state = COORDINATOR_ABORT;
            return;
        }
    }
    c->state = COORDINATOR_COMMIT;
}

/* Broadcast final decision */
void coordinator_send_decision(coordinator_t *c) {
    if (c->state != COORDINATOR_COMMIT && c->state != COORDINATOR_ABORT) return;
    message_t decision = (c->state == COORDINATOR_COMMIT) ? MSG_COMMIT : MSG_ABORT;
    for (size_t i = 0; i < c->count; ++i) {
        participant_receive_message(c->participants[i], decision);
    }
}
