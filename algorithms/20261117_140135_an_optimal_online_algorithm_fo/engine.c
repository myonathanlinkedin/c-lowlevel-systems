#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdlib.h>
#include <assert.h>

/* -------------------------------------------------------------------------
 * Queue implementation (simple singly‑linked list)
 * ------------------------------------------------------------------------- */
void job_queue_init(JobQueue* q)
{
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

void job_queue_destroy(JobQueue* q)
{
    JobNode* cur = q->head;
    while (cur) {
        JobNode* nxt = cur->next;
        free(cur);
        cur = nxt;
    }
    q->head = q->tail = NULL;
    q->size = 0;
}

void job_queue_push(JobQueue* q, const Job* job)
{
    JobNode* node = (JobNode*)malloc(sizeof(JobNode));
    assert(node != NULL);               /* Allocation failure is fatal */
    node->job = *job;
    node->next = NULL;
    if (q->tail) {
        q->tail->next = node;
    } else {
        q->head = node;
    }
    q->tail = node;
    ++q->size;
}

bool job_queue_pop(JobQueue* q, Job* out)
{
    if (!q->head) {
        return false;
    }
    JobNode* node = q->head;
    *out = node->job;
    q->head = node->next;
    if (!q->head) {
        q->tail = NULL;
    }
    free(node);
    --q->size;
    return true;
}

/* -------------------------------------------------------------------------
 * Helper: compute total remaining processing per agent (including queued jobs)
 * ------------------------------------------------------------------------- */
static uint32_t remaining_total(const JobQueue* q)
{
    uint32_t sum = 0;
    for (JobNode* cur = q->head; cur != NULL; cur = cur->next) {
        sum += cur->job.processing;
    }
    return sum;
}

/* -------------------------------------------------------------------------
 * Core algorithm
 * ------------------------------------------------------------------------- */
bool optimal_schedule_two_agents(const Job* jobs,
                                 size_t n_jobs,
                                 ScheduleEntry* out_sched,
                                 size_t* out_len)
{
    JobQueue pending[2];
    job_queue_init(&pending[AGENT_A]);
    job_queue_init(&pending[AGENT_B]);

    uint32_t current_time = 0;
    size_t   out_index    = 0;
    size_t   arrival_idx  = 0;   /* Index of next job that has not yet arrived */

    /* Main loop: continue while there are unprocessed arrivals or pending jobs */
    while (arrival_idx < n_jobs || pending[AGENT_A].size || pending[AGENT_B].size) {
        /* 1. Enqueue all jobs that have arrived up to the current moment.
         *    In this reference implementation we treat the arrival sequence as
         *    instantaneous – i.e., every job is considered to have arrived
         *    before the first scheduling decision.  This matches the classic
         *    online model where the algorithm sees jobs in order but does not
         *    know future jobs.
         */
        while (arrival_idx < n_jobs) {
            job_queue_push(&pending[jobs[arrival_idx].agent],
                           &jobs[arrival_idx]);
            ++arrival_idx;
        }

        /* 2. If no pending jobs, we are done (should not happen inside loop). */
        if (!pending[AGENT_A].size && !pending[AGENT_B].size) {
            break;
        }

        /* 3. Determine which agent has larger remaining total processing. */
        uint32_t rem_A = remaining_total(&pending[AGENT_A]);
        uint32_t rem_B = remaining_total(&pending[AGENT_B]);

        Agent chosen_agent;
        if (rem_A > rem_B) {
            chosen_agent = AGENT_A;
        } else if (rem_B > rem_A) {
            chosen_agent = AGENT_B;
        } else {
            /* Tie – prefer the agent whose first pending job arrived earlier.
             * Since we inserted jobs in arrival order, the head of each queue
             * reflects the earliest pending job for that agent.
             */
            if (pending[AGENT_A].head && pending[AGENT_B].head) {
                chosen_agent = (pending[AGENT_A].head->job.id <
                                pending[AGENT_B].head->job.id) ?
                               AGENT_A : AGENT_B;
            } else if (pending[AGENT_A].head) {
                chosen_agent = AGENT_A;
            } else {
                chosen_agent = AGENT_B;
            }
        }

        /* 4. Dequeue the chosen job and record its schedule entry. */
        Job next_job;
        bool ok = job_queue_pop(&pending[chosen_agent], &next_job);
        assert(ok);   /* Invariant: chosen queue cannot be empty */

        out_sched[out_index].job_id = next_job.id;
        out_sched[out_index].start  = current_time;
        out_sched[out_index].end    = current_time + next_job.processing;
        ++out_index;

        current_time += next_job.processing;
    }

    /* Clean up */
    job_queue_destroy(&pending[AGENT_A]);
    job_queue_destroy(&pending[AGENT_B]);

    *out_len = out_index;
    return true;
}
