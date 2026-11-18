#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <assert.h>

/* ---------- Coordinator implementation ---------- */
void coordinator_init(coordinator_t *coord) {
    assert(coord != NULL);
    coord->participant_count = 0;
    coord->state = COORDINATOR_IDLE;
    for (size_t i = 0; i < MAX_PARTICIPANTS; ++i) {
        coord->participants[i] = NULL;
    }
}

bool coordinator_add_participant(coordinator_t *coord, participant_t *p) {
    assert(coord != NULL);
    assert(p != NULL);
    if (coord->participant_count >= MAX_PARTICIPANTS) {
        return false;
    }
    coord->participants[coord->participant_count++] = p;
    p->state = PARTICIPANT_INIT;
    return true;
}

/* Reset participant to initial state – useful for repeated tests */
void participant_reset(participant_t *p) {
    assert(p != NULL);
    p->state = PARTICIPANT_INIT;
}

/* Configure the participant's vote for the upcoming transaction */
void participant_set_vote(participant_t *p, bool commit) {
    assert(p != NULL);
    p->vote_commit = commit;
}

/* ---------- Core two‑phase commit logic ---------- */

/* Phase 1 – ask each participant to prepare */
void coordinator_process_prepare(coordinator_t *coord) {
    assert(coord != NULL);
    coord->state = COORDINATOR_PREPARING;
    for (size_t i = 0; i < coord->participant_count; ++i) {
        participant_t *p = coord->participants[i];
        assert(p != NULL);
        /* Simulate sending a PREPARE request */
        p->state = PARTICIPANT_PREPARED;
    }
}

/* Phase 2 – decide based on votes and broadcast decision */
void coordinator_process_decision(coordinator_t *coord, bool all_commit) {
    assert(coord != NULL);
    if (all_commit) {
        coord->state = COORDINATOR_COMMITTING;
        for (size_t i = 0; i < coord->participant_count; ++i) {
            participant_t *p = coord->participants[i];
            assert(p != NULL);
            p->state = PARTICIPANT_COMMITTED;
        }
    } else {
        coord->state = COORDINATOR_ABORTING;
        for (size_t i = 0; i < coord->participant_count; ++i) {
            participant_t *p = coord->participants[i];
            assert(p != NULL);
            p->state = PARTICIPANT_ABORTED;
        }
    }
}

/* Public entry point – runs a full transaction */
void coordinator_start_transaction(coordinator_t *coord) {
    assert(coord != NULL);
    if (coord->participant_count == 0) {
        /* Trivial transaction – nothing to do */
        coord->state = COORDINATOR_COMMITTING;
        return;
    }

    coordinator_process_prepare(coord);

    /* Collect votes */
    bool all_commit = true;
    for (size_t i = 0; i < coord->participant_count; ++i) {
        participant_t *p = coord->participants[i];
        assert(p != NULL);
        if (!p->vote_commit) {
            all_commit = false;
            break;
        }
    }

    coordinator_process_decision(coord, all_commit);
    coord->state = COORDINATOR_IDLE;   /* Ready for next transaction */
}
