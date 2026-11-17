#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdio.h>
#include <assert.h>

/* Helper to print a schedule (useful for debugging) */
static void print_schedule(const ScheduleEntry* sched, size_t len)
{
    printf("Schedule (%zu entries):\n", len);
    for (size_t i = 0; i < len; ++i) {
        printf("  Job %d: [%u, %u)\n",
               sched[i].job_id,
               sched[i].start,
               sched[i].end);
    }
}

/* Verify that the makespan (completion time of the last job) equals the sum
 * of all processing times – a basic sanity check for a single‑machine schedule.
 */
static void assert_makespan(const ScheduleEntry* sched,
                            size_t len,
                            uint32_t total_processing)
{
    if (len == 0) {
        assert(total_processing == 0);
        return;
    }
    uint32_t makespan = sched[len - 1].end;
    assert(makespan == total_processing);
}

/* Compute total processing per agent from the original job list */
static void compute_totals(const Job* jobs,
                           size_t n,
                           uint32_t* out_A,
                           uint32_t* out_B)
{
    uint32_t sum_A = 0, sum_B = 0;
    for (size_t i = 0; i < n; ++i) {
        if (jobs[i].agent == AGENT_A) {
            sum_A += jobs[i].processing;
        } else {
            sum_B += jobs[i].processing;
        }
    }
    *out_A = sum_A;
    *out_B = sum_B;
}

/* Unit test 1: empty input */
static void test_empty(void)
{
    Job jobs[1]; /* unused */
    ScheduleEntry sched[1];
    size_t out_len = 0;
    bool ok = optimal_schedule_two_agents(jobs, 0, sched, &out_len);
    assert(ok);
    assert(out_len == 0);
}

/* Unit test 2: single job */
static void test_single(void)
{
    Job jobs[1] = { { .id = 1, .agent = AGENT_A, .processing = 7 } };
    ScheduleEntry sched[1];
    size_t out_len = 0;
    bool ok = optimal_schedule_two_agents(jobs, 1, sched, &out_len);
    assert(ok);
    assert(out_len == 1);
    assert(sched[0].job_id == 1);
    assert(sched[0].start == 0);
    assert(sched[0].end == 7);
    assert_makespan(sched, out_len, 7);
}

/* Unit test 3: two agents, equal total load */
static void test_equal_load(void)
{
    Job jobs[4] = {
        { .id = 1, .agent = AGENT_A, .processing = 3 },
        { .id = 2, .agent = AGENT_B, .processing = 5 },
        { .id = 3, .agent = AGENT_A, .processing = 2 },
        { .id = 4, .agent = AGENT_B, .processing = 0 } /* zero‑time job (edge) */
    };
    ScheduleEntry sched[4];
    size_t out_len = 0;
    bool ok = optimal_schedule_two_agents(jobs, 4, sched, &out_len);
    assert(ok);
    assert(out_len == 4);

    uint32_t total_A, total_B;
    compute_totals(jobs, 4, &total_A, &total_B);
    assert(total_A == 5);
    assert(total_B == 5);

    /* The makespan must be sum of all processing times (10). */
    assert_makespan(sched, out_len, total_A + total_B);
}

/* Unit test 4: unbalanced loads – algorithm should prioritize the heavier side */
static void test_unbalanced(void)
{
    Job jobs[5] = {
        { .id = 1, .agent = AGENT_A, .processing = 4 },
        { .id = 2, .agent = AGENT_B, .processing = 1 },
        { .id = 3, .agent = AGENT_A, .processing = 3 },
        { .id = 4, .agent = AGENT_B, .processing = 2 },
        { .id = 5, .agent = AGENT_A, .processing = 2 }
    };
    ScheduleEntry sched[5];
    size_t out_len = 0;
    bool ok = optimal_schedule_two_agents(jobs, 5, sched, &out_len);
    assert(ok);
    assert(out_len == 5);

    uint32_t total_A, total_B;
    compute_totals(jobs, 5, &total_A, &total_B);
    assert(total_A == 9);
    assert(total_B == 3);

    /* Verify that the first three scheduled jobs belong to AGENT_A,
     * because its remaining load dominates until the balance flips. */
    assert(sched[0].job_id == 1); /* A */
    assert(sched[1].job_id == 3); /* A */
    assert(sched[2].job_id == 5); /* A */
    /* After A's load drops to 2, B becomes heavier (remaining 3) */
    assert(sched[3].job_id == 2); /* B */
    assert(sched[4].job_id == 4); /* B */

    assert_makespan(sched, out_len, total_A + total_B);
}

/* Unit test 5: tie‑breaking by earliest arrival */
static void test_tie_break(void)
{
    Job jobs[2] = {
        { .id = 10, .agent = AGENT_A, .processing = 5 },
        { .id = 5,  .agent = AGENT_B, .processing = 5 }
    };
    ScheduleEntry sched[2];
    size_t out_len = 0;
    bool ok = optimal_schedule_two_agents(jobs, 2, sched, &out_len);
    assert(ok);
    assert(out_len == 2);
    /* Totals are equal; the algorithm must pick the job with smaller id
     * (earlier arrival) first. */
    assert(sched[0].job_id == 10);
    assert(sched[1].job_id == 5);
    assert_makespan(sched, out_len, 10);
}

/* -------------------------------------------------------------------------
 * Main driver – runs all unit tests and prints a demonstration schedule.
 * ------------------------------------------------------------------------- */
int main(void)
{
    test_empty();
    test_single();
    test_equal_load();
    test_unbalanced();
    test_tie_break();

    /* Demonstration with a mixed set of jobs */
    Job demo_jobs[6] = {
        { .id = 1, .agent = AGENT_A, .processing = 4 },
        { .id = 2, .agent = AGENT_B, .processing = 6 },
        { .id = 3, .agent = AGENT_A, .processing = 3 },
        { .id = 4, .agent = AGENT_B, .processing = 2 },
        { .id = 5, .agent = AGENT_A, .processing = 5 },
        { .id = 6, .agent = AGENT_B, .processing = 1 }
    };
    ScheduleEntry demo_sched[6];
    size_t demo_len = 0;
    bool ok = optimal_schedule_two_agents(demo_jobs, 6, demo_sched, &demo_len);
    assert(ok);
    print_schedule(demo_sched, demo_len);

    return 0;
}
