#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdatomic.h>
#include <assert.h>
#include <pthread.h>

typedef struct {
    uint64_t *buf;
    size_t capacity;      // must be power of two
    size_t mask;
    atomic_size_t head;   // write index
    atomic_size_t tail;   // read index
} ring_buffer_t;

/* Initialize ring buffer. capacity must be power of two and >0 */
static int ring_buffer_init(ring_buffer_t *rb, size_t capacity) {
    if (capacity == 0 || (capacity & (capacity - 1)) != 0) return -1;
    rb->buf = (uint64_t *)malloc(sizeof(uint64_t) * capacity);
    if (!rb->buf) return -1;
    rb->capacity = capacity;
    rb->mask = capacity - 1;
    atomic_init(&rb->head, 0);
    atomic_init(&rb->tail, 0);
    return 0;
}

static void ring_buffer_free(ring_buffer_t *rb) {
    free(rb->buf);
    rb->buf = NULL;
}

/* Push value into buffer. Returns 0 on success, -1 if full */
static int ring_buffer_push(ring_buffer_t *rb, uint64_t val) {
    size_t head = atomic_load_explicit(&rb->head, memory_order_relaxed);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    if ((head - tail) == rb->capacity) return -1; // full
    rb->buf[head & rb->mask] = val;
    atomic_store_explicit(&rb->head, head + 1, memory_order_release);
    return 0;
}

/* Pop value from buffer. Returns 0 on success, -1 if empty */
static int ring_buffer_pop(ring_buffer_t *rb, uint64_t *out) {
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_relaxed);
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    if (head == tail) return -1; // empty
    *out = rb->buf[tail & rb->mask];
    atomic_store_explicit(&rb->tail, tail + 1, memory_order_release);
    return 0;
}

/* ---------- Test harness ---------- */

#define TEST_ITEMS 1000000
#define RING_CAP   1024

typedef struct {
    ring_buffer_t *rb;
    size_t count;
    int   result;
} thread_arg_t;

static void *producer(void *arg) {
    thread_arg_t *a = (thread_arg_t *)arg;
    size_t i = 0;
    while (i < a->count) {
        if (ring_buffer_push(a->rb, (uint64_t)i) == 0) {
            ++i;
        } else {
            /* buffer full, spin */
            sched_yield();
        }
    }
    a->result = 0;
    return NULL;
}

static void *consumer(void *arg) {
    thread_arg_t *a = (thread_arg_t *)arg;
    uint64_t val;
    size_t i = 0;
    while (i < a->count) {
        if (ring_buffer_pop(a->rb, &val) == 0) {
            assert(val == (uint64_t)i);
            ++i;
        } else {
            /* buffer empty, spin */
            sched_yield();
        }
    }
    a->result = 0;
    return NULL;
}

int main(void) {
    ring_buffer_t rb;
    assert(ring_buffer_init(&rb, RING_CAP) == 0);

    /* Single-threaded sanity test */
    for (size_t i = 0; i < RING_CAP; ++i) {
        assert(ring_buffer_push(&rb, i) == 0);
    }
    assert(ring_buffer_push(&rb, 0) == -1); // should be full
    for (size_t i = 0; i < RING_CAP; ++i) {
        uint64_t v;
        assert(ring_buffer_pop(&rb, &v) == 0);
        assert(v == i);
    }
    assert(ring_buffer_pop(&rb, &(uint64_t){0}) == -1); // should be empty

    /* Multi-threaded producer/consumer test */
    thread_arg_t prod = { .rb = &rb, .count = TEST_ITEMS, .result = -1 };
    thread_arg_t cons = { .rb = &rb, .count = TEST_ITEMS, .result = -1 };
    pthread_t pt, ct;
    assert(pthread_create(&pt, NULL, producer, &prod) == 0);
    assert(pthread_create(&ct, NULL, consumer, &cons) == 0);
    assert(pthread_join(pt, NULL) == 0);
    assert(pthread_join(ct, NULL) == 0);
    assert(prod.result == 0 && cons.result == 0);

    ring_buffer_free(&rb);
    printf("All tests passed.\n");
    return 0;
}
