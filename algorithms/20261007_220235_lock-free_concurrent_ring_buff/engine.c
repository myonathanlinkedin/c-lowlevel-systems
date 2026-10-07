#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <string.h>

void ring_init(RingBuffer* rb) {
    if (rb == NULL) return;
    memset(rb->data, 0, sizeof(rb->data));
    rb->head = 0;
    rb->tail = 0;
}

bool ring_push(RingBuffer* rb, uint64_t value) {
    if (rb == NULL) return false;
    
    uint32_t head = atomic_load_explicit(&rb->head, memory_order_relaxed);
    uint32_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    
    if ((head + 1) % RING_CAPACITY == tail) {
        return false; // Buffer is full
    }
    
    if (!atomic_compare_exchange_weak_explicit(&rb->head, &head, head + 1, memory_order_acq_rel, memory_order_relaxed)) {
        return false; // Another thread pushed
    }
    
    rb->data[head % RING_CAPACITY] = value;
    return true;
}

bool ring_pop(RingBuffer* rb, uint64_t* value) {
    if (rb == NULL || value == NULL) return false;
    
    uint32_t tail = atomic_load_explicit(&rb->tail, memory_order_relaxed);
    uint32_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    
    if (tail == head) {
        return false; // Buffer is empty
    }
    
    if (!atomic_compare_exchange_weak_explicit(&rb->tail, &tail, tail + 1, memory_order_acq_rel, memory_order_relaxed)) {
        return false; // Another thread popped
    }
    
    *value = rb->data[tail % RING_CAPACITY];
    return true;
}
