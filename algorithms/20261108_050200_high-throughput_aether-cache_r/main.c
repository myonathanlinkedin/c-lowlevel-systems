#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#include "types.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    aether_ring_t rb;
    const size_t cap = 8;               /* Power‑of‑two capacity */

    /* Initialise */
    assert(aether_ring_init(&rb, cap) && "Ring buffer initialisation failed");
    assert(aether_ring_is_empty(&rb));
    assert(!aether_ring_is_full(&rb));
    assert(aether_ring_size(&rb) == 0);

    /* Fill the buffer to capacity‑1 (one slot is always left free to distinguish full/empty) */
    for (uint8_t i = 0; i < (uint8_t)(cap - 1); ++i) {
        assert(aether_ring_push(&rb, i) && "Push should succeed");
        assert(aether_ring_size(&rb) == i + 1);
    }
    assert(!aether_ring_is_empty(&rb));
    assert(aether_ring_is_full(&rb));
    assert(aether_ring_size(&rb) == cap - 1);

    /* Attempt to push when full – must fail */
    assert(!aether_ring_push(&rb, 0xFF) && "Push on full buffer must fail");

    /* Pop all elements and verify order */
    for (uint8_t i = 0; i < (uint8_t)(cap - 1); ++i) {
        uint8_t val = 0xAA;
        assert(aether_ring_pop(&rb, &val) && "Pop should succeed");
        assert(val == i && "FIFO order violated");
        assert(aether_ring_size(&rb) == (cap - 1) - (i + 1));
    }
    assert(aether_ring_is_empty(&rb));
    assert(!aether_ring_is_full(&rb));
    assert(aether_ring_size(&rb) == 0);

    /* Test wrap‑around behaviour */
    for (uint8_t i = 0; i < 4; ++i) {
        assert(aether_ring_push(&rb, i + 100));
    }
    for (uint8_t i = 0; i < 2; ++i) {
        uint8_t val;
        assert(aether_ring_pop(&rb, &val));
        assert(val == i + 100);
    }
    for (uint8_t i = 0; i < 4; ++i) {
        assert(aether_ring_push(&rb, i + 200));
    }
    /* At this point the internal indices have wrapped; verify remaining order */
    uint8_t expected[] = {102, 103, 200, 201, 202, 203};
    for (size_t i = 0; i < sizeof(expected); ++i) {
        uint8_t val;
        assert(aether_ring_pop(&rb, &val));
        assert(val == expected[i]);
    }
    assert(aether_ring_is_empty(&rb));

    /* Invalid initialisation (non‑power‑of‑two) must fail */
    aether_ring_t bad;
    assert(!aether_ring_init(&bad, 7) && "Non‑power‑of‑two capacity should be rejected");

    aether_ring_free(&rb);
    aether_ring_free(&bad);   /* Safe even if allocation never occurred */

    printf("All Aether‑Cache ring‑buffer tests passed.\n");
    return 0;
}
