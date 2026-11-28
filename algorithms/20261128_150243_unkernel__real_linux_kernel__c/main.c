#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>

#include "types.h"
#include <stdio.h>
#include <assert.h>

/* Global scheduler instance for the demo */
static scheduler_t g_scheduler;

/* -------------------------------------------------------------------------- */
/* Test task 1 – increments a counter 5 times, then finishes */
static int counter_a = 0;
static void task_a(void)
{
    ++counter_a;
    if (counter_a >= 5) {
        scheduler_finish_current();
        return;
    }
    scheduler_yield();
}

/* Test task 2 – increments a counter 3 times, then finishes */
static int counter_b = 0;
static void task_b(void)
{
    ++counter_b;
    if (counter_b >= 3) {
        scheduler_finish_current();
        return;
    }
    scheduler_yield();
}

/* -------------------------------------------------------------------------- */
int main(void)
{
    /* Initialise scheduler with capacity for 4 tasks */
    scheduler_init(&g_scheduler, 4);

    /* Basic sanity checks */
    assert(scheduler_add_task(&g_scheduler, task_a) == true);
    assert(scheduler_add_task(&g_scheduler, task_b) == true);

    /* Adding a NULL task must fail */
    assert(scheduler_add_task(&g_scheduler, NULL) == false);

    /* Fill the scheduler to capacity and verify overflow handling */
    assert(scheduler_add_task(&g_scheduler, task_a) == true);
    assert(scheduler_add_task(&g_scheduler, task_b) == true);
    /* Scheduler is now full – next addition should be rejected */
    assert(scheduler_add_task(&g_scheduler, task_a) == false);

    /* Run the scheduler – enough ticks to let all tasks finish */
    scheduler_run(&g_scheduler, 100);

    /* Verify that counters have reached their expected values */
    assert(counter_a == 5);
    assert(counter_b == 3);

    /* After all tasks have finished, the queue must be empty */
    assert(g_scheduler.count == 0);
    assert(g_scheduler.head == g_scheduler.tail);

    /* Clean up */
    scheduler_destroy(&g_scheduler);

    /* If we reach this point, all assertions passed */
    printf("All unit tests passed.\n");
    return 0;
}
