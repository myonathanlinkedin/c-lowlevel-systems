
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------
 * Enumerations
 * ------------------------------------------------------------------------- */
typedef enum {
    AGENT_A = 0,
    AGENT_B = 1
} Agent;

/* -------------------------------------------------------------------------
 * Job definition
 * ------------------------------------------------------------------------- */
typedef struct {
    int          id;          /* Unique identifier (for debugging) */
    Agent        agent;       /* Owner of the job */
    uint32_t     processing; /* Processing time (positive) */
} Job;

/* -------------------------------------------------------------------------
 * Schedule entry (result of the algorithm)
 * ------------------------------------------------------------------------- */
typedef struct {
    int          job_id;      /* Job identifier */
    uint32_t     start;       /* Start time (inclusive) */
    uint32_t     end;         /* End time (exclusive) */
} ScheduleEntry;

/* -------------------------------------------------------------------------
 * Simple FIFO queue for pending jobs (per agent)
 * ------------------------------------------------------------------------- */
typedef struct JobNode {
    Job                 job;
    struct JobNode*     next;
} JobNode;

typedef struct {
    JobNode*    head;
    JobNode*    tail;
    size_t      size;
} JobQueue;

/* -------------------------------------------------------------------------
 * Public API (engine.c)
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* Initialise an empty queue */
void job_queue_init(JobQueue* q);

/* Release all nodes owned by the queue */
void job_queue_destroy(JobQueue* q);

/* Enqueue a job (copy) */
void job_queue_push(JobQueue* q, const Job* job);

/* Dequeue a job; returns false if queue empty */
bool job_queue_pop(JobQueue* q, Job* out);

/* Core scheduling routine.
 * Input:
 *   jobs      – array of jobs in arrival order
 *   n_jobs    – number of jobs in the array
 * Output:
 *   out_sched – caller‑allocated array where schedule entries are written
 *   out_len   – receives the number of entries written (must be ≤ n_jobs)
 *
 * The algorithm is the optimal online algorithm for a single machine with
 * two agents: at each idle moment it selects a pending job belonging to the
 * agent whose *remaining total processing time* (including the job itself)
 * is larger.  Ties are broken by preferring the earlier‑arrived job.
 *
 * The function returns true on success, false on allocation failure.
 */
bool optimal_schedule_two_agents(const Job* jobs,
                                 size_t n_jobs,
                                 ScheduleEntry* out_sched,
                                 size_t* out_len);

#ifdef __cplusplus
}
#endif
