#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int main() {
    RingBuffer rb;
    ring_init(&rb);
    
    // Test 1: Push to empty buffer
    assert(ring_push(&rb, 1) == true);
    assert(ring_push(&rb, 2) == true);
    
    // Test 2: Pop from buffer
    uint64_t val;
    assert(ring_pop(&rb, &val) == true);
    assert(val == 1);
    assert(ring_pop(&rb, &val) == true);
    assert(val == 2);
    
    // Test 3: Pop from empty buffer
    assert(ring_pop(&rb, &val) == false);
    
    // Test 4: Fill buffer to capacity
    for (int i = 0; i < RING_CAPACITY - 1; i++) {
        assert(ring_push(&rb, (uint64_t)i) == true);
    }
    
    // Test 5: Buffer is now full
    assert(ring_push(&rb, 999) == false);
    
    // Test 6: Pop all elements
    for (int i = 0; i < RING_CAPACITY - 1; i++) {
        assert(ring_pop(&rb, &val) == true);
        assert(val == (uint64_t)i);
    }
    
    // Test 7: Buffer is empty again
    assert(ring_pop(&rb, &val) == false);
    
    // Test 8: Wrap-around test
    for (int i = 0; i < RING_CAPACITY - 1; i++) {
        assert(ring_push(&rb, (uint64_t)(i + 100)) == true);
    }
    assert(ring_push(&rb, 999) == false);
    
    for (int i = 0; i < RING_CAPACITY - 1; i++) {
        assert(ring_pop(&rb, &val) == true);
        assert(val == (uint64_t)(i + 100));
    }
    
    // Test 9: NULL pointer safety
    assert(ring_push(NULL, 1) == false);
    assert(ring_pop(NULL, &val) == false);
    assert(ring_pop(&rb, NULL) == false);
    
    printf("All tests passed.\n");
    return 0;
}
