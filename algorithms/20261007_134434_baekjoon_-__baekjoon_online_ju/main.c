#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include "types.h"
#include "core.h"

int main(void) {
    FenwickTree ft;
    fenwick_init(&ft, 10);

    for (size_t i = 1; i <= 10; ++i) {
        fenwick_update(&ft, i, 0);
    }

    fenwick_update(&ft, 3, 5);
    fenwick_update(&ft, 5, 2);
    fenwick_update(&ft, 7, 7);

    assert(fenwick_query(&ft, 3) == 5);
    assert(fenwick_query(&ft, 5) == 2);
    assert(fenwick_query(&ft, 7) == 7);

    assert(fenwick_query(&ft, 3) == 5);
    assert(fenwick_query(&ft, 5) == 7);
    assert(fenwick_query(&ft, 7) == 14);

    assert(fenwick_range_query(&ft, 1, 3) == 5);
    assert(fenwick_range_query(&ft, 3, 5) == 7);
    assert(fenwick_range_query(&ft, 5, 7) == 9);
    assert(fenwick_range_query(&ft, 1, 10) == 14);
    assert(fenwick_range_query(&ft, 4, 3) == 0);
    assert(fenwick_range_query(&ft, 1, ft.size) == 14);

    for (size_t i = 1; i <= 10; ++i) {
        fenwick_update(&ft, i, (int64_t)i);
    }

    int64_t expected_prefix = 0;
    for (size_t i = 1; i <= 10; ++i) {
        expected_prefix += (int64_t)i;
        int64_t base = (i == 3 ? 5 : (i == 5 ? 2 : (i == 7 ? 7 : 0)));
        assert(fenwick_query(&ft, i) == expected_prefix + base);
    }

    fenwick_destroy(&ft);
    printf("All tests passed.\n");
    return 0;
}
