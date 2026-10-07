#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>
#include "types.h"

// Define a function to initialize the HyperLogLog counter
void hll_init(hll_counter_t* counter) {
    hll_init(counter);
}

// Define a function to increment the HyperLogLog counter
void hll_inc(hll_counter_t* counter, uint64_t value) {
    hll_inc(counter, value);
}

// Define a function to count the number of distinct elements in a HyperLogLog counter
uint64_t hll_count(hll_counter_t counter) {
    uint64_t count = hll_count(counter);
    return count;
}
