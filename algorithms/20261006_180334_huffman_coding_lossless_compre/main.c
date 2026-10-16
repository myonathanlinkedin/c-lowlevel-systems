#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include "core.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static void test_roundtrip(const char *input) {
    size_t len = strlen(input);
    HuffmanTree *tree = build_huffman_tree((const uint8_t *)input, len);
    assert(tree != NULL);
    HuffmanCode codes[256] = {0};
    generate_codes(tree->root, codes, 0, 0);
    BitWriter bw;
    bool ok = encode((const uint8_t *)input, len, codes, &bw);
    assert(ok);
    uint8_t decoded[1024];
    size_t decoded_len = 0;
    BitReader br;
    bitreader_init(&br, bw.buffer, bw.size);
    ok = decode(&br, tree, decoded, &decoded_len);
    assert(ok);
    assert(decoded_len == len);
    assert(memcmp(input, decoded, len) == 0);
    bitwriter_free(&bw);
    free_huffman_tree(tree);
}

int main(void) {
    test_roundtrip("");
    test_roundtrip("a");
    test_roundtrip("aaaaaa");
    test_roundtrip("hello world");
    test_roundtrip("The quick brown fox jumps over the lazy dog");
    printf("All tests passed.\n");
    return 0;
}
