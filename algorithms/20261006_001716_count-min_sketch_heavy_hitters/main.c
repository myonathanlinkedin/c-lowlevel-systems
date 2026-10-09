#include <stdint.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "types.h"

int main() {
    CountMinSketch sketch;

    count_min_sketch_init(&sketch, 1000000);

    for (int i = 0; i < 1000000; i++) {
        uint64_t key = i;
        uint64_t value = rand();
        count_min_sketch_update(&sketch, key, value);
    }

    uint64_t result = count_min_sketch_query(&sketch, 500000);

    if (result == 0) {
        printf("Key 500000 is a heavy hitter!\n");
    } else {
        printf("Key 500000 is not a heavy hitter (%llu)\n", result);
    }

    return 0;
}
