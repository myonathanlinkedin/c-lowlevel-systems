#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include "types.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static void test_roundtrip(const uint8_t* data, size_t len)
{
    uint8_t* comp = NULL;
    size_t comp_len = 0;
    bool ok = huffman_compress(data, len, &comp, &comp_len);
    assert(ok);
    assert(comp != NULL);
    assert(comp_len > 0);

    uint8_t* decomp = NULL;
    size_t decomp_len = 0;
    ok = huffman_decompress(comp, comp_len, &decomp, &decomp_len);
    assert(ok);
    assert(decomp != NULL);
    assert(decomp_len == len);
    assert(memcmp(data, decomp, len) == 0);

    huffman_free(comp);
    huffman_free(decomp);
}

int main(void)
{
    /* Empty input */
    test_roundtrip((const uint8_t*)"", 0);

    /* Single character repeated */
    const char* s1 = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    test_roundtrip((const uint8_t*)s1, strlen(s1));

    /* Typical English sentence */
    const char* s2 = "The quick brown fox jumps over the lazy dog.";
    test_roundtrip((const uint8_t*)s2, strlen(s2));

    /* All possible byte values */
    uint8_t all[256];
    for (int i = 0; i < 256; ++i) all[i] = (uint8_t)i;
    test_roundtrip(all, 256);

    /* Random data (deterministic) */
    uint8_t rnd[1024];
    uint32_t seed = 0xDEADBEEF;
    for (size_t i = 0; i < sizeof(rnd); ++i) {
        seed = seed * 1664525u + 1013904223u;
        rnd[i] = (uint8_t)(seed >> 24);
    }
    test_roundtrip(rnd, sizeof(rnd));

    printf("All Huffman compression/decompression tests passed.\n");
    return 0;
}
