#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>

#include "types.h"
#include "core.h"
#include <assert.h>
#include <stdio.h>

static void test_successful_commit(void) {
    coordinator_t *coord = coordinator_create();
    assert(coord != NULL);
    participant_t *p1 = participant_create(1, true);
    participant_t *p2 = participant_create(2, true);
    participant_t *p3 = participant_create(3, true);
    assert(p1 && p2 && p3);
    assert(coordinator_add_participant(coord, p1));
    assert(coordinator_add_participant(coord, p2));
    assert(coordinator_add_participant(coord, p3));
    assert(coordinator_start(coord));
    assert(coordinator_collect_votes(coord) == VOTE_COMMIT);
    assert(coordinator_decide(coord) == true);
    assert(coord->state == COORD_DONE);
    assert(p1->state == PART_COMMITTED);
    assert(p2->state == PART_COMMITTED);
    assert(p3->state == PART_COMMITTED);
    coordinator_destroy(coord);
    participant_destroy(p1);
    participant_destroy(p2);
    participant_destroy(p3);
}

static void test_abort_due_to_negative_vote(void) {
    coordinator_t *coord = coordinator_create();
    participant_t *p1 = participant_create(1, true);
    participant_t *p2 = participant_create(2, false); /* votes no */
    participant_t *p3 = participant_create(3, true);
    assert(coordinator_add_participant(coord, p1));
    assert(coordinator_add_participant(coord, p2));
    assert(coordinator_add_participant(coord, p3));
    assert(coordinator_start(coord));
    assert(coordinator_collect_votes(coord) == VOTE_ABORT);
    assert(coordinator_decide(coord) == false);
    assert(coord->state == COORD_DONE);
    assert(p1->state == PART_ABORTED);
    assert(p2->state == PART_ABORTED);
    assert(p3->state == PART_ABORTED);
    coordinator_destroy(coord);
    participant_destroy(p1);
    participant_destroy(p2);
    participant_destroy(p3);
}

static void test_no_participants(void) {
    coordinator_t *coord = coordinator_create();
    assert(coord != NULL);
    assert(coordinator_start(coord));          /* No participants, still moves to PREPARE */
    assert(coordinator_collect_votes(coord) == VOTE_COMMIT); /* Vacuously commit */
    assert(coordinator_decide(coord) == true);
    assert(coord->state == COORD_DONE);
    coordinator_destroy(coord);
}

int main(void) {
    test_successful_commit();
    test_abort_due_to_negative_vote();
    test_no_participants();
    printf("All Two-Phase Commit tests passed.\n");
    return 0;
}
