#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>

#include "types.h"
#include <assert.h>
#include <stdio.h>

/* ---------- Test helpers ---------- */

/* A simple counter task.
 * The task decrements a counter until it reaches zero.
 * Returns true while work remains, false when done.
 */
typedef struct {
    int *counter;
    int id;
} CounterTaskArg;

static bool counter_task(void *arg) {
    CounterTaskArg *cta = (CounterTaskArg *)arg;
    if (*(cta->counter) > 0) {
        (*(cta->counter))--;
        /* For demonstration we could print, but keep output minimal. */
        return true; /* More work to do */
    }
    return false;    /* Completed */
}

/* ---------- Unit tests ---------- */

static void test_single_task(void) {
    int count = 5;
    CounterTaskArg arg = { &count, 1 };
    Task *t = task_create(counter_task, &arg);
    assert(t != NULL);
    scheduler_init();
    scheduler_add(t);
    scheduler_run();
    assert(count == 0);
    assert(scheduler_task_count() == 0);
}

static void test_multiple_tasks_round_robin(void) {
    int c1 = 3, c2 = 5, c3 = 2;
    CounterTaskArg a1 = { &c1, 1 };
    CounterTaskArg a2 = { &c2, 2 };
    CounterTaskArg a3 = { &c3, 3 };
    Task *t1 = task_create(counter_task, &a1);
    Task *t2 = task_create(counter_task, &a2);
    Task *t3 = task_create(counter_task, &a3);
    assert(t1 && t2 && t3);
    scheduler_init();
    scheduler_add(t1);
    scheduler_add(t2);
    scheduler_add(t3);
    scheduler_run();
    assert(c1 == 0 && c2 == 0 && c3 == 0);
    assert(scheduler_task_count() == 0);
}

/* Verify that adding a NULL task is caught by the assert in scheduler_add. */
static void test_invalid_add(void) {
    scheduler_init();
    /* The following line is intentionally commented out because it would abort.
       Uncomment to see the assert in action.
    scheduler_add(NULL);
    */
}

/* ---------- Main driver ---------- */
int main(void) {
    test_single_task();
    test_multiple_tasks_round_robin();
    /* test_invalid_add(); // not executed to keep test suite non‑terminating */
    printf("All tests passed.\n");
    return 0;
}
