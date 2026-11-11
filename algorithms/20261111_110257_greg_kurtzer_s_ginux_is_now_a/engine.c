#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdlib.h>
#include <assert.h>

/* Internal representation of a task. */
struct Task {
    TaskFunc func;   /* Task function */
    void *arg;       /* Argument passed to func */
    bool finished;   /* Cached completion flag */
};

/* Simple dynamic array based queue for tasks. */
typedef struct {
    Task **data;     /* Array of pointers to Task */
    size_t capacity;/* Allocated size of data */
    size_t size;    /* Number of active tasks */
    size_t head;    /* Index of the next task to run */
} TaskQueue;

/* Scheduler state – a single global queue. */
static TaskQueue g_queue = {NULL, 0, 0, 0};

/* Internal helper: ensure the queue has enough capacity. */
static void queue_ensure_capacity(void) {
    if (g_queue.size < g_queue.capacity) {
        return;
    }
    size_t new_cap = (g_queue.capacity == 0) ? 4 : g_queue.capacity * 2;
    Task **new_data = (Task **)realloc(g_queue.data, new_cap * sizeof(Task *));
    assert(new_data != NULL);
    g_queue.data = new_data;
    g_queue.capacity = new_cap;
}

/* Internal helper: push a task to the tail of the queue. */
static void queue_push(Task *t) {
    assert(t != NULL);
    queue_ensure_capacity();
    size_t tail = (g_queue.head + g_queue.size) % g_queue.capacity;
    g_queue.data[tail] = t;
    g_queue.size++;
}

/* Internal helper: pop the task at the head of the queue. */
static Task *queue_pop(void) {
    if (g_queue.size == 0) {
        return NULL;
    }
    Task *t = g_queue.data[g_queue.head];
    g_queue.head = (g_queue.head + 1) % g_queue.capacity;
    g_queue.size--;
    return t;
}

/* Public API implementations */

Task *task_create(TaskFunc func, void *arg) {
    if (func == NULL) {
        return NULL;
    }
    Task *t = (Task *)malloc(sizeof(Task));
    if (t == NULL) {
        return NULL;
    }
    t->func = func;
    t->arg = arg;
    t->finished = false;
    return t;
}

void task_destroy(Task *t) {
    if (t != NULL) {
        free(t);
    }
}

/* Scheduler initialization – resets the global queue. */
void scheduler_init(void) {
    free(g_queue.data);
    g_queue.data = NULL;
    g_queue.capacity = 0;
    g_queue.size = 0;
    g_queue.head = 0;
}

/* Add a task to the scheduler. */
void scheduler_add(Task *t) {
    assert(t != NULL);
    queue_push(t);
}

/* Run tasks in a round‑robin fashion until all are finished. */
void scheduler_run(void) {
    while (g_queue.size > 0) {
        Task *t = queue_pop();
        if (t == NULL) {
            continue;
        }
        if (!t->finished) {
            bool more = t->func(t->arg);
            t->finished = !more;
        }
        if (!t->finished) {
            /* Re‑queue unfinished tasks. */
            queue_push(t);
        } else {
            /* Task completed – destroy it. */
            task_destroy(t);
        }
    }
}

/* Return the current number of queued tasks (useful for assertions). */
size_t scheduler_task_count(void) {
    return g_queue.size;
}
