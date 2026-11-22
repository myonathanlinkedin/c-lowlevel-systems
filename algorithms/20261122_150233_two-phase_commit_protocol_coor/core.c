#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include "types.h"
#include <stdlib.h>
#include <stdio.h>

/* ---------- Participant implementation ---------- */
static bool participant_receive_prepare_impl(participant_t *self) {
    if (self->state != PART_INIT) return false;
    self->state = PART_READY;
    return self->vote_yes;   /* true = vote yes, false = vote no */
}
static void participant_receive_commit_impl(participant_t *self) {
    if (self->state == PART_READY) {
        self->state = PART_COMMITTED;
    }
}
static void participant_receive_abort_impl(participant_t *self) {
    if (self->state == PART_READY || self->state == PART_INIT) {
        self->state = PART_ABORTED;
    }
}
static participant_vtable_t participant_vtable = {
    .self = NULL,
    .receive_prepare = participant_receive_prepare_impl,
    .receive_commit = participant_receive_commit_impl,
    .receive_abort = participant_receive_abort_impl
};

participant_t *participant_create(uint32_t id, bool vote_yes) {
    participant_t *p = (participant_t *)malloc(sizeof(participant_t));
    if (!p) return NULL;
    p->id = id;
    p->state = PART_INIT;
    p->vote_yes = vote_yes;
    p->vtable = &participant_vtable;
    return p;
}
void participant_destroy(participant_t *p) {
    free(p);
}
bool participant_receive_prepare(participant_t *p) {
    return p->vtable->receive_prepare(p);
}
void participant_receive_commit(participant_t *p) {
    p->vtable->receive_commit(p);
}
void participant_receive_abort(participant_t *p) {
    p->vtable->receive_abort(p);
}

/* ---------- Coordinator implementation ---------- */
coordinator_t *coordinator_create(void) {
    coordinator_t *c = (coordinator_t *)malloc(sizeof(coordinator_t));
    if (!c) return NULL;
    c->state = COORD_INIT;
    c->participants = NULL;
    c->participant_cnt = 0;
    c->votes_yes = 0;
    c->votes_no = 0;
    return c;
}
void coordinator_destroy(coordinator_t *c) {
    if (c) {
        free(c->participants);
        free(c);
    }
}
bool coordinator_add_participant(coordinator_t *c, participant_t *p) {
    if (!c || c->state != COORD_INIT) return false;
    size_t new_cnt = c->participant_cnt + 1;
    participant_t **tmp = (participant_t **)realloc(c->participants,
                                                    new_cnt * sizeof(participant_t *));
    if (!tmp) return false;
    c->participants = tmp;
    c->participants[c->participant_cnt] = p;
    c->participant_cnt = new_cnt;
    return true;
}
bool coordinator_start(coordinator_t *c) {
    if (!c || c->state != COORD_INIT) return false;
    c->state = COORD_PREPARE;
    c->votes_yes = 0;
    c->votes_no = 0;
    for (size_t i = 0; i < c->participant_cnt; ++i) {
        bool vote = participant_receive_prepare(c->participants[i]);
        if (vote) {
            ++c->votes_yes;
        } else {
            ++c->votes_no;
        }
    }
    return true;
}
vote_result_t coordinator_collect_votes(coordinator_t *c) {
    if (!c || c->state != COORD_PREPARE) return VOTE_PENDING;
    if (c->votes_no > 0) {
        c->state = COORD_ABORT;
        return VOTE_ABORT;
    }
    if (c->votes_yes == c->participant_cnt) {
        c->state = COORD_COMMIT;
        return VOTE_COMMIT;
    }
    return VOTE_PENDING;
}
bool coordinator_decide(coordinator_t *c) {
    if (!c) return false;
    vote_result_t vr = coordinator_collect_votes(c);
    if (vr == VOTE_COMMIT) {
        for (size_t i = 0; i < c->participant_cnt; ++i) {
            participant_receive_commit(c->participants[i]);
        }
        c->state = COORD_DONE;
        return true;
    } else if (vr == VOTE_ABORT) {
        for (size_t i = 0; i < c->participant_cnt; ++i) {
            participant_receive_abort(c->participants[i]);
        }
        c->state = COORD_DONE;
        return false;
    }
    return false;   /* Should not reach here */
}
