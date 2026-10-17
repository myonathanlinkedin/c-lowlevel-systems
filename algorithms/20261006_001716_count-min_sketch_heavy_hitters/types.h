#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint64_t hash_table[256];
    uint64_t count;
} CountMinSketch;

#endif //TYPES_H
