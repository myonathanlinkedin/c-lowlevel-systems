#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/* Global pointer used by yield/finish helpers */
static scheduler_t *g_sched = NULL;

/* -------------------------------------------------------------------------- */
void scheduler_init(scheduler_t *sched, size_t capacity)
{
    assert(sched != NULL);
    assert(capacity > 0);

    sched->queue = (task_t **)calloc(capacity, sizeof(task_t *));
    assert(sched->queue != NULL);

    sched->capacity = capacity;
    sched->head = 0;
    sched->tail = 0;
    sched->count = 0;
    sched->next_id = 1;
    sched->current = NULL;
    g_sched = sched;
}

/* -------------------------------------------------------------------------- */
static bool queue_is_full(const scheduler_t *sched)
{
    return sched->count == sched->capacity;
}

/* -------------------------------------------------------------------------- */
static bool queue_is_empty(const scheduler_t *sched)
{
    return sched->count == 0;
}

/* -------------------------------------------------------------------------- */
bool scheduler_add_task(scheduler_t *sched, task_func_t func)
{
    if (sched == NULL || func == NULL)
        return false;
    if (queue_is_full(sched))
        return false;

    task_t *t = (task_t *)malloc(sizeof(task_t));
    if (t == NULL)
        return false;

    t->id = sched->next_id++;
    t->func = func;
    t->state = TASK_READY;
    t->context = NULL;

    sched->queue[sched->tail] = t;
    sched->tail = (sched->tail + 1) % sched->capacity;
    ++sched->count;
    return true;
}

/* -------------------------------------------------------------------------- */
static void dequeue_current(scheduler_t *sched)
{
    /* Remove the task at head (already executed) */
    assert(sched->count > 0);
    task_t *t = sched->queue[sched->head];
    free(t);
    sched->queue[sched->head] = NULL;
    sched->head = (sched->head + 1) % sched->capacity;
    --sched->count;
}

/* -------------------------------------------------------------------------- */
static void requeue_current(scheduler_t *sched)
{
    /* Move head forward, then place the task at tail */
    task_t *t = sched->queue[sched->head];
    sched->queue[sched->head] = NULL;
    sched->head = (sched->head + 1) % sched->capacity;
    /* Insert at tail */
    sched->queue[sched->tail] = t;
    sched->tail = (sched->tail + 1) % sched->capacity;
    /* count unchanged */
}

/* -------------------------------------------------------------------------- */
void scheduler_run(scheduler_t *sched, size_t max_ticks)
{
    assert(sched != NULL);
    size_t ticks = 0;

    while (!queue_is_empty(sched) && ticks < max_ticks) {
        task_t *t = sched->queue[sched->head];
        assert(t != NULL);
        t->state = TASK_RUNNING;
        sched->current = t;
        g_sched = sched;               /* Ensure helpers see the current scheduler */

        /* Execute the task */
        t->func();

        /* After return, decide what to do */
        if (t->state == TASK_FINISHED) {
            dequeue_current(sched);
        } else {
            t->state = TASK_READY;
            requeue_current(sched);
        }

        ++ticks;
    }
}

/* -------------------------------------------------------------------------- */
void scheduler_yield(void)
{
    /* Cooperative yield – simply return to the scheduler.
       No state change required; the scheduler will re‑queue the task. */
    return;
}

/* -------------------------------------------------------------------------- */
void scheduler_finish_current(void)
{
    if (g_sched == NULL || g_sched->current == NULL)
        return;
    g_sched->current->state = TASK_FINISHED;
}

/* -------------------------------------------------------------------------- */
task_t* scheduler_current_task(void)
{
    if (g_sched == NULL)
        return NULL;
    return g_sched->current;
}

/* -------------------------------------------------------------------------- */
void scheduler_destroy(scheduler_t *sched)
{
    if (sched == NULL)
        return;

    /* Free any remaining tasks */
    while (!queue_is_empty(sched)) {
        task_t *t = sched->queue[sched->head];
        free(t);
        sched->queue[sched->head] = NULL;
        sched->head = (sched->head + 1) % sched->capacity;
        --sched->count;
    }

    free(sched->queue);
    sched->queue = NULL;
    sched->capacity = 0;
    sched->head = sched->tail = 0;
    sched->next_id = 0;
    sched->current = NULL;
    g_sched = NULL;
}
