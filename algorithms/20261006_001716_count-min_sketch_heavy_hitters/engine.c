#include <stdint.h>
#include <string.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "types.h"

void count_min_sketch_init(CountMinSketch* sketch, uint64_t size) {
    sketch->count = 0;
    memset(sketch->hash_table, 0, sizeof(sketch->hash_table));
}

void count_min_sketch_update(CountMinSketch* sketch, uint64_t key, uint64_t value) {
    sketch->hash_table[key] += value;
    sketch->count += value;
}

uint64_t count_min_sketch_query(CountMinSketch* sketch, uint64_t key) {
    return sketch->hash_table[key] + sketch->count - sketch->hash_table[0];
}
