#include "types.h"
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct TemporalQueue {
    TemporalItem* data;   // binary min‑heap array
    size_t        size;   // number of valid elements
    size_t        cap;    // allocated capacity
};

/* ---------- internal helpers ---------- */

static void swap_items(TemporalItem* a, TemporalItem* b) {
    TemporalItem tmp = *a;
    *a = *b;
    *b = tmp;
}

/* heapify up from index i (0‑based) */
static void heapify_up(TemporalQueue* q, size_t i) {
    while (i != 0) {
        size_t parent = (i - 1U) >> 1U;
        if (q->data[parent].timestamp <= q->data[i].timestamp) break;
        swap_items(&q->data[parent], &q->data[i]);
        i = parent;
    }
}

/* heapify down from index i (0‑based) */
static void heapify_down(TemporalQueue* q, size_t i) {
    size_t n = q->size;
    while (true) {
        size_t left  = (i << 1U) + 1U;
        size_t right = left + 1U;
        size_t smallest = i;

        if (left < n && q->data[left].timestamp < q->data[smallest].timestamp)
            smallest = left;
        if (right < n && q->data[right].timestamp < q->data[smallest].timestamp)
            smallest = right;

        if (smallest == i) break;
        swap_items(&q->data[i], &q->data[smallest]);
        i = smallest;
    }
}

/* ensure capacity for at least one more element */
static bool ensure_capacity(TemporalQueue* q) {
    if (q->size < q->cap) return true;
    size_t new_cap = (q->cap == 0) ? 4U : q->cap * 2U;
    TemporalItem* tmp = (TemporalItem*)realloc(q->data, new_cap * sizeof(TemporalItem));
    if (!tmp) return false;
    q->data = tmp;
    q->cap  = new_cap;
    return true;
}

/* ---------- public API ---------- */

TemporalQueue* tq_create(void) {
    TemporalQueue* q = (TemporalQueue*)malloc(sizeof(TemporalQueue));
    if (!q) return NULL;
    q->data = NULL;
    q->size = 0;
    q->cap  = 0;
    return q;
}

void tq_destroy(TemporalQueue* q) {
    if (!q) return;
    free(q->data);
    free(q);
}

bool tq_push(TemporalQueue* q, uint64_t timestamp, int value) {
    if (!q) return false;
    if (!ensure_capacity(q)) return false;
    q->data[q->size].timestamp = timestamp;
    q->data[q->size].value     = value;
    heapify_up(q, q->size);
    q->size++;
    return true;
}

bool tq_pop(TemporalQueue* q, uint64_t* out_timestamp, int* out_value) {
    if (!q || q->size == 0) return false;
    if (out_timestamp) *out_timestamp = q->data[0].timestamp;
    if (out_value)     *out_value     = q->data[0].value;

    q->size--;
    if (q->size > 0) {
        q->data[0] = q->data[q->size];
        heapify_down(q, 0);
    }
    return true;
}

bool tq_peek(const TemporalQueue* q, uint64_t* out_timestamp, int* out_value) {
    if (!q || q->size == 0) return false;
    if (out_timestamp) *out_timestamp = q->data[0].timestamp;
    if (out_value)     *out_value     = q->data[0].value;
    return true;
}

void tq_purge_before(TemporalQueue* q, uint64_t cutoff) {
    if (!q) return;
    // Repeatedly pop while the smallest timestamp is < cutoff.
    while (q->size > 0 && q->data[0].timestamp < cutoff) {
        q->size--;
        if (q->size > 0) {
            q->data[0] = q->data[q->size];
            heapify_down(q, 0);
        }
    }
}

size_t tq_size(const TemporalQueue* q) {
    return q ? q->size : 0;
}
