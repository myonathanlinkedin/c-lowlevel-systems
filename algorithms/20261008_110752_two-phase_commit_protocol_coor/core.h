#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#pragma once
#include "types.h"

typedef struct participant participant_t;
typedef struct coordinator coordinator_t;

/* Participant API */
participant_t *participant_create(int id);
void participant_destroy(participant_t *p);
void participant_set_vote(participant_t *p, bool vote_commit);
bool participant_get_vote(const participant_t *p);
participant_state_t participant_get_state(const participant_t *p);
void participant_receive_message(participant_t *p, message_t msg);

/* Coordinator API */
coordinator_t *coordinator_create(participant_t **participants, size_t count);
void coordinator_destroy(coordinator_t *c);
coordinator_state_t coordinator_get_state(const coordinator_t *c);
void coordinator_start_transaction(coordinator_t *c);
void coordinator_process_responses(coordinator_t *c);
void coordinator_send_decision(coordinator_t *c);
