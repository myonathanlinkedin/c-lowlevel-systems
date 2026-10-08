#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void test_all_commit(void) {
    const size_t n = 3;
    participant_t *parts[n];
    for (size_t i = 0; i < n; ++i) {
        parts[i] = participant_create((int)i);
        participant_set_vote(parts[i], true);   // all vote commit
    }

    coordinator_t *coord = coordinator_create(parts, n);
    coordinator_start_transaction(coord);
    coordinator_process_responses(coord);
    assert(coordinator_get_state(coord) == COORDINATOR_COMMIT);
    coordinator_send_decision(coord);

    for (size_t i = 0; i < n; ++i) {
        assert(participant_get_state(parts[i]) == PARTICIPANT_COMMITTED);
        participant_destroy(parts[i]);
    }
    coordinator_destroy(coord);
    printf("test_all_commit passed\n");
}

static void test_one_abort(void) {
    const size_t n = 4;
    participant_t *parts[n];
    for (size_t i = 0; i < n; ++i) {
        parts[i] = participant_create((int)i);
        participant_set_vote(parts[i], i != 2); // participant 2 votes abort
    }

    coordinator_t *coord = coordinator_create(parts, n);
    coordinator_start_transaction(coord);
    coordinator_process_responses(coord);
    assert(coordinator_get_state(coord) == COORDINATOR_ABORT);
    coordinator_send_decision(coord);

    for (size_t i = 0; i < n; ++i) {
        assert(participant_get_state(parts[i]) == PARTICIPANT_ABORTED);
        participant_destroy(parts[i]);
    }
    coordinator_destroy(coord);
    printf("test_one_abort passed\n");
}

static void test_state_transitions(void) {
    participant_t *p = participant_create(0);
    coordinator_t *c = coordinator_create(&p, 1);

    assert(participant_get_state(p) == PARTICIPANT_INIT);
    assert(coordinator_get_state(c) == COORDINATOR_INIT);

    coordinator_start_transaction(c);
    assert(coordinator_get_state(c) == COORDINATOR_WAITING);
    assert(participant_get_state(p) == PARTICIPANT_PREPARED);

    coordinator_process_responses(c);
    assert(coordinator_get_state(c) == COORDINATOR_COMMIT);

    coordinator_send_decision(c);
    assert(participant_get_state(p) == PARTICIPANT_COMMITTED);

    participant_destroy(p);
    coordinator_destroy(c);
    printf("test_state_transitions passed\n");
}

int main(void) {
    test_all_commit();
    test_one_abort();
    test_state_transitions();
    printf("All tests passed.\n");
    return 0;
}
