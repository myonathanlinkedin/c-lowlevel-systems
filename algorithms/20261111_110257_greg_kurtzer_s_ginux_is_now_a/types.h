
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#pragma once

#include <stdbool.h>
#include <stddef.h>

/* Forward declaration of the Task structure */
typedef struct Task Task;

/* Function pointer type for a task.
 * The function receives a user-provided argument and returns:
 *   true  – the task has more work to do and should be rescheduled.
 *   false – the task is complete.
 */
typedef bool (*TaskFunc)(void *arg);

/* Public API */

/* Create a new task.
 * Parameters:
 *   func – the function implementing the task logic.
 *   arg  – opaque pointer passed to func on each invocation.
 * Returns:
 *   Pointer to a newly allocated Task, or NULL on allocation failure.
 */
Task *task_create(TaskFunc func, void *arg);

/* Destroy a task previously created with task_create.
 * The function frees all resources owned by the task.
 */
void task_destroy(Task *t);

/* Initialize the scheduler.
 * Must be called before any other scheduler function.
 */
void scheduler_init(void);

/* Add a task to the scheduler queue.
 * Parameters:
 *   t – pointer to a Task; must not be NULL.
 * The scheduler takes ownership of the pointer; the caller must not free it.
 */
void scheduler_add(Task *t);

/* Run the scheduler until all queued tasks have completed.
 * The function returns after the task queue becomes empty.
 */
void scheduler_run(void);

/* Query the number of tasks currently queued (for testing purposes). */
size_t scheduler_task_count(void);
