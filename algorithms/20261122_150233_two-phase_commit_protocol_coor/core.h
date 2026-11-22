#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#pragma once
#include "types.h"

/* Coordinator API */
coordinator_t *coordinator_create(void);
void coordinator_destroy(coordinator_t *c);
bool coordinator_add_participant(coordinator_t *c, participant_t *p);
bool coordinator_start(coordinator_t *c);          /* Begin 2PC */
vote_result_t coordinator_collect_votes(coordinator_t *c);
bool coordinator_decide(coordinator_t *c);         /* Returns true if commit */

/* Participant API */
participant_t *participant_create(uint32_t id, bool vote_yes);
void participant_destroy(participant_t *p);
bool participant_receive_prepare(participant_t *p);
void participant_receive_commit(participant_t *p);
void participant_receive_abort(participant_t *p);
