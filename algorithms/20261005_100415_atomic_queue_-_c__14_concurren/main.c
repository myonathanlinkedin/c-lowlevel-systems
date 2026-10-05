/*=== main.c =============================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <threads.h>

typedef struct {
    size_t          capacity;      /* must be power of two */
    size_t          mask;
    _Atomic size_t *seq;           /* per-slot sequence numbers */
    void           **buffer;       /* slots */
    _Atomic size_t  head;
    _Atomic size_t  tail;
} mpmc_queue_t;

/* Utility: round up to next power of two (>=1) */
static size_t next_pow2(size_t v) {
    if (v == 0) return 1;
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
#if SIZE_MAX > UINT32_MAX
    v |= v >> 32;
#endif
    return v + 1;
}

/* Initialise queue; returns 0 on success, -1 on failure */
static int mpmc_queue_init(mpmc_queue_t *q, size_t capacity) {
    if (!q) return -1;
    capacity = next_pow2(capacity);
    q->capacity = capacity;
    q->mask = capacity - 1;
    q->buffer = calloc(capacity, sizeof(void *));
    q->seq    = calloc(capacity, sizeof(_Atomic size_t));
    if (!q->buffer || !q->seq) {
        free(q->buffer);
        free(q->seq);
        return -1;
    }
    for (size_t i = 0; i < capacity; ++i) {
        atomic_init(&q->seq[i], i);
    }
    atomic_init(&q->head, 0);
    atomic_init(&q->tail, 0);
    return 0;
}

/* Destroy queue */
static void mpmc_queue_destroy(mpmc_queue_t *q) {
    if (!q) return;
    free(q->buffer);
    free(q->seq);
}

/* Enqueue item; returns 1 on success, 0 if full */
static int mpmc_queue_enqueue(mpmc_queue_t *q, void *data) {
    size_t pos, idx, seq;
    intptr_t dif;
    while (1) {
        pos = atomic_load_explicit(&q->tail, memory_order_relaxed);
        idx = pos & q->mask;
        seq = atomic_load_explicit(&q->seq[idx], memory_order_acquire);
        dif = (intptr_t)seq - (intptr_t)pos;
        if (dif == 0) {
            if (atomic_compare_exchange_weak_explicit(
                    &q->tail, &pos, pos + 1,
                    memory_order_relaxed, memory_order_relaxed)) {
                q->buffer[idx] = data;
                atomic_store_explicit(&q->seq[idx], pos + 1, memory_order_release);
                return 1;
            }
        } else if (dif < 0) {
            return 0; /* full */
        } else {
            /* another producer moved tail, retry */
        }
    }
}

/* Dequeue item; returns 1 on success, 0 if empty */
static int mpmc_queue_dequeue(mpmc_queue_t *q, void **data) {
    size_t pos, idx, seq;
    intptr_t dif;
    while (1) {
        pos = atomic_load_explicit(&q->head, memory_order_relaxed);
        idx = pos & q->mask;
        seq = atomic_load_explicit(&q->seq[idx], memory_order_acquire);
        dif = (intptr_t)seq - (intptr_t)(pos + 1);
        if (dif == 0) {
            if (atomic_compare_exchange_weak_explicit(
                    &q->head, &pos, pos + 1,
                    memory_order_relaxed, memory_order_relaxed)) {
                *data = q->buffer[idx];
                atomic_store_explicit(&q->seq[idx], pos + q->mask + 1,
                                      memory_order_release);
                return 1;
            }
        } else if (dif < 0) {
            return 0; /* empty */
        } else {
            /* another consumer moved head, retry */
        }
    }
}

/*-------------------------- Unit Tests -----------------------------------*/

static int test_basic(void) {
    mpmc_queue_t q;
    assert(mpmc_queue_init(&q, 8) == 0);

    /* Enqueue 0..7 */
    for (int i = 0; i < 8; ++i) {
        int *v = malloc(sizeof(int));
        *v = i;
        assert(mpmc_queue_enqueue(&q, v) == 1);
    }
    /* Queue should be full now */
    int dummy = 42;
    assert(mpmc_queue_enqueue(&q, &dummy) == 0);

    /* Dequeue and verify order */
    for (int i = 0; i < 8; ++i) {
        int *v = NULL;
        assert(mpmc_queue_dequeue(&q, (void **)&v) == 1);
        assert(v && *v == i);
        free(v);
    }
    /* Queue should be empty now */
    int *out = (int *)0x1;
    assert(mpmc_queue_dequeue(&q, (void **)&out) == 0);
    mpmc_queue_destroy(&q);
    return 0;
}

/* Multi‑threaded producer/consumer test */
#define MT_ITEMS 100000
#define MT_THREADS 4

typedef struct {
    mpmc_queue_t *q;
    int start;
} prod_arg_t;

typedef struct {
    mpmc_queue_t *q;
    atomic_int *counter;
} cons_arg_t;

static int producer(void *arg) {
    prod_arg_t *pa = (prod_arg_t *)arg;
    for (int i = 0; i < MT_ITEMS; ++i) {
        int *v = malloc(sizeof(int));
        *v = pa->start + i;
        while (!mpmc_queue_enqueue(pa->q, v)) {
            thrd_yield();
        }
    }
    return 0;
}

static int consumer(void *arg) {
    cons_arg_t *ca = (cons_arg_t *)arg;
    int *v;
    int received = 0;
    while (received < MT_ITEMS) {
        if (mpmc_queue_dequeue(ca->q, (void **)&v)) {
            free(v);
            atomic_fetch_add(ca->counter, 1);
            ++received;
        } else {
            thrd_yield();
        }
    }
    return 0;
}

static int test_mt(void) {
    mpmc_queue_t q;
    assert(mpmc_queue_init(&q, 1024) == 0);
    atomic_int total = ATOMIC_VAR_INIT(0);
    thrd_t prod[MT_THREADS];
    thrd_t cons[MT_THREADS];
    prod_arg_t pargs[MT_THREADS];
    cons_arg_t cargs[MT_THREADS];

    for (int i = 0; i < MT_THREADS; ++i) {
        pargs[i].q = &q;
        pargs[i].start = i * MT_ITEMS;
        thrd_create(&prod[i], producer, &pargs[i]);
        cargs[i].q = &q;
        cargs[i].counter = &total;
        thrd_create(&cons[i], consumer, &cargs[i]);
    }
    for (int i = 0; i < MT_THREADS; ++i) {
        thrd_join(prod[i], NULL);
        thrd_join(cons[i], NULL);
    }
    assert(atomic_load(&total) == MT_THREADS * MT_ITEMS);
    mpmc_queue_destroy(&q);
    return 0;
}

/*-------------------------- Main Driver ---------------------------------*/

int main(void) {
    printf("Running basic test... ");
    fflush(stdout);
    test_basic();
    printf("OK\n");

    printf("Running multithreaded test... ");
    fflush(stdout);
    test_mt();
    printf("OK\n");

    printf("All tests passed.\n");
    return 0;
}
