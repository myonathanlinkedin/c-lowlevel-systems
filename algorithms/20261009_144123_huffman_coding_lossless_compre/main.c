#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#include "core.h"
#include "types.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void test_roundtrip(const uint8_t *data, size_t len) {
    uint32_t freq[256] = {0};
    for (size_t i = 0; i < len; ++i) freq[data[i]]++;

    HuffmanNode *root = NULL;
    huffman_build_tree(freq, &root);
    HuffmanCode codes[256];
    huffman_generate_codes(root, codes);

    uint8_t comp[1024] = {0};
    size_t comp_len = huffman_compress(data, len, comp, codes);

    uint8_t decomp[1024] = {0};
    size_t decomp_len = huffman_decompress(comp, comp_len, decomp, sizeof(decomp), root);

    assert(decomp_len == len);
    assert(memcmp(data, decomp, len) == 0);

    huffman_free_tree(root);
}

int main(void) {
    /* Test 1: typical sentence */
    const char *msg = "this is an example for huffman encoding";
    test_roundtrip((const uint8_t *)msg, strlen(msg));

    /* Test 2: empty input */
    test_roundtrip((const uint8_t *)"", 0);

    /* Test 3: single repeated symbol */
    uint8_t repeated[100];
    memset(repeated, 'A', sizeof(repeated));
    test_roundtrip(repeated, sizeof(repeated));

    /* Test 4: all possible byte values once */
    uint8_t all[256];
    for (int i = 0; i < 256; ++i) all[i] = (uint8_t)i;
    test_roundtrip(all, sizeof(all));

    printf("All Huffman tests passed.\n");
    return 0;
}
