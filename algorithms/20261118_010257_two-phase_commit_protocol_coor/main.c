#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>

#include "types.h"
#include <stdio.h>
#include <assert.h>

static void test_all_commit(void) {
    coordinator_t coord;
    participant_t p1 = { .name = "P1" };
    participant_t p2 = { .name = "P2" };
    participant_t p3 = { .name = "P3" };

    coordinator_init(&coord);
    assert(coordinator_add_participant(&coord, &p1));
    assert(coordinator_add_participant(&coord, &p2));
    assert(coordinator_add_participant(&coord, &p3));

    participant_set_vote(&p1, true);
    participant_set_vote(&p2, true);
    participant_set_vote(&p3, true);

    coordinator_start_transaction(&coord);

    assert(coord.state == COORDINATOR_IDLE);
    assert(participant_get_state(&p1) == PARTICIPANT_COMMITTED);
    assert(participant_get_state(&p2) == PARTICIPANT_COMMITTED);
    assert(participant_get_state(&p3) == PARTICIPANT_COMMITTED);
}

static void test_one_abort(void) {
    coordinator_t coord;
    participant_t p1 = { .name = "P1" };
    participant_t p2 = { .name = "P2" };
    participant_t p3 = { .name = "P3" };

    coordinator_init(&coord);
    assert(coordinator_add_participant(&coord, &p1));
    assert(coordinator_add_participant(&coord, &p2));
    assert(coordinator_add_participant(&coord, &p3));

    participant_set_vote(&p1, true);
    participant_set_vote(&p2, false);   /* This participant aborts */
    participant_set_vote(&p3, true);

    coordinator_start_transaction(&coord);

    assert(coord.state == COORDINATOR_IDLE);
    assert(participant_get_state(&p1) == PARTICIPANT_ABORTED);
    assert(participant_get_state(&p2) == PARTICIPANT_ABORTED);
    assert(participant_get_state(&p3) == PARTICIPANT_ABORTED);
}

static void test_all_abort(void) {
    coordinator_t coord;
    participant_t p1 = { .name = "P1" };
    participant_t p2 = { .name = "P2" };

    coordinator_init(&coord);
    assert(coordinator_add_participant(&coord, &p1));
    assert(coordinator_add_participant(&coord, &p2));

    participant_set_vote(&p1, false);
    participant_set_vote(&p2, false);

    coordinator_start_transaction(&coord);

    assert(coord.state == COORDINATOR_IDLE);
    assert(participant_get_state(&p1) == PARTICIPANT_ABORTED);
    assert(participant_get_state(&p2) == PARTICIPANT_ABORTED);
}

static void test_zero_participants(void) {
    coordinator_t coord;
    coordinator_init(&coord);
    coordinator_start_transaction(&coord);
    /* With no participants the coordinator should end in COMMITTING state
       (nothing to abort). */
    assert(coord.state == COORDINATOR_COMMITTING);
}

int main(void) {
    test_all_commit();
    test_one_abort();
    test_all_abort();
    test_zero_participants();

    /* If we reach this point, all assertions passed */
    printf("All Two‑Phase Commit tests passed.\n");
    return 0;
}
