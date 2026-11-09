#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "types.h"
#include "core.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    /* Basic creation and empty‑check */
    MinHeap *h = heap_create();
    assert(h != NULL);
    assert(heap_is_empty(h));
    assert(heap_size(h) == 0);

    /* Insert a known sequence and verify ordering */
    int seq[] = {5, 3, 8, 1, 9, 2};
    for (size_t i = 0; i < sizeof(seq) / sizeof(seq[0]); ++i) {
        assert(heap_push(h, seq[i]));
    }
    assert(!heap_is_empty(h));
    assert(heap_size(h) == 6);

    int top;
    assert(heap_peek(h, &top));
    assert(top == 1);

    int expected[] = {1, 2, 3, 5, 8, 9};
    for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        int val;
        assert(heap_pop(h, &val));
        assert(val == expected[i]);
    }
    assert(heap_is_empty(h));
    assert(heap_size(h) == 0);

    /* Pop from empty heap must fail */
    int dummy;
    assert(!heap_pop(h, &dummy));

    /* Stress test: push 1000 elements in reverse order to trigger resizing */
    for (int i = 1000; i >= 1; --i) {
        assert(heap_push(h, i));
    }
    assert(heap_size(h) == 1000);
    for (int i = 1; i <= 1000; ++i) {
        int v;
        assert(heap_pop(h, &v));
        assert(v == i);
    }
    assert(heap_is_empty(h));

    heap_destroy(h);
    printf("All tests passed.\n");
    return 0;
}
