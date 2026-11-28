
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/* Task function type */
typedef void (*task_func_t)(void);

/* Task execution state */
typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_FINISHED
} task_state_t;

/* Task descriptor */
typedef struct {
    uint32_t    id;      /* Unique identifier */
    task_func_t func;    /* Function to execute */
    task_state_t state;  /* Current state */
    void       *context; /* Optional user data (unused in this demo) */
} task_t;

/* Simple round‑robin scheduler */
typedef struct {
    task_t   **queue;      /* Circular buffer of task pointers */
    size_t     capacity;   /* Maximum number of tasks */
    size_t     head;       /* Index of next task to run */
    size_t     tail;       /* Index for inserting new tasks */
    size_t     count;      /* Number of tasks currently queued */
    uint32_t   next_id;    /* Generator for task IDs */
    task_t    *current;    /* Task currently executing (NULL if idle) */
} scheduler_t;

/* Scheduler public API */
void scheduler_init(scheduler_t *sched, size_t capacity);
bool scheduler_add_task(scheduler_t *sched, task_func_t func);
void scheduler_run(scheduler_t *sched, size_t max_ticks);
void scheduler_yield(void);
void scheduler_finish_current(void);
task_t* scheduler_current_task(void);
void scheduler_destroy(scheduler_t *sched);
