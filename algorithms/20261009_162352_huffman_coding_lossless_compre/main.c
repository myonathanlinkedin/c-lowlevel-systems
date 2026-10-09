#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "types.h"
#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

static int g_failures = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { printf("FAIL: %s\n", msg); g_failures++; } \
    else { printf("PASS: %s\n", msg); } \
} while (0)

static void test_single_symbol(void) {
    uint8_t data[10];
    for (int i = 0; i < 10; i++) data[i] = 'A';
    HuffTree t;
    huff_build(data, 10, &t);
    CHECK(t.root >= 0, "single-symbol tree built");
    CHECK(t.lengths['A'] == 1, "single-symbol code length is 1");
    uint8_t bits[16];
    size_t nb = huff_encode(data, 10, &t, bits, sizeof(bits));
    CHECK(nb == 2, "single-symbol encodes to 2 bytes");
    uint8_t dec[16];
    size_t nd = huff_decode(bits, 10, &t, dec, sizeof(dec));
    CHECK(nd == 10 && memcmp(dec, data, 10) == 0, "single-symbol roundtrip");
}

static void test_two_symbols(void) {
    uint8_t data[8] = {'a','b','a','a','b','a','a','a'};
    HuffTree t;
    huff_build(data, 8, &t);
    CHECK(t.lengths['a'] == 1 && t.lengths['b'] == 1, "two-symbol lengths");
    uint8_t bits[16];
    size_t nb = huff_encode(data, 8, &t, bits, sizeof(bits));
    CHECK(nb == 1, "two-symbol encodes to 1 byte");
    uint8_t dec[16];
    size_t nd = huff_decode(bits, 8, &t, dec, sizeof(dec));
    CHECK(nd == 8 && memcmp(dec, data, 8) == 0, "two-symbol roundtrip");
}

static void test_roundtrip_text(void) {
    const char *msg = "Huffman coding is a lossless compression algorithm.";
    size_t len = strlen(msg);
    uint8_t data[256];
    memcpy(data, msg, len);
    HuffTree t;
    huff_build(data, len, &t);
    uint8_t bits[512];
    size_t nb = huff_encode(data, len, &t, bits, sizeof(bits));
    CHECK(nb > 0, "text encodes");
    uint8_t dec[256];
    size_t nd = huff_decode(bits, nb * 8, &t, dec, sizeof(dec));
    CHECK(nd == len && memcmp(dec, data, len) == 0, "text roundtrip");
    size_t orig_bits = len * 8;
    size_t enc_bits = huff_bit_length(data, len, &t);
    CHECK(enc_bits < orig_bits, "compression reduces bit count");
    printf("  (orig=%zu bits, encoded=%zu bits, ratio=%.2f)\n",
           orig_bits, enc_bits, (double)enc_bits / (double)orig_bits);
}

static void test_all_symbols(void) {
    uint8_t data[HUFF_ALPHABET];
    for (int i = 0; i < HUFF_ALPHABET; i++) data[i] = (uint8_t)i;
    HuffTree t;
    huff_build(data, HUFF_ALPHABET, &t);
    uint8_t bits[512];
    size_t nb = huff_encode(data, HUFF_ALPHABET, &t, bits, sizeof(bits));
    CHECK(nb > 0, "all-symbols encodes");
    uint8_t dec[HUFF_ALPHABET];
    size_t nd = huff_decode(bits, nb * 8, &t, dec, sizeof(dec));
    CHECK(nd == HUFF_ALPHABET && memcmp(dec, data, HUFF_ALPHABET) == 0,
          "all-symbols roundtrip");
}

static void test_empty(void) {
    HuffTree t;
    huff_build(NULL, 0, &t);
    CHECK(t.root == -1, "empty input yields no root");
    uint8_t bits[8], dec[8];
    size_t nb = huff_encode(NULL, 0, &t, bits, sizeof(bits));
    CHECK(nb == 0, "empty encodes to 0 bytes");
    size_t nd = huff_decode(bits, 0, &t, dec, sizeof(dec));
    CHECK(nd == 0, "empty decodes to 0");
}

static void test_capacity_guard(void) {
    uint8_t data[64];
    for (int i = 0; i < 64; i++) data[i] = (uint8_t)(i % 4);
    HuffTree t;
    huff_build(data, 64, &t);
    uint8_t bits[1];
    size_t nb = huff_encode(data, 64, &t, bits, 1);
    CHECK(nb == 0, "encode returns 0 when buffer too small");
}

static void test_benchmark(void) {
    size_t N = 100000;
    uint8_t *data = (uint8_t *)malloc(N);
    srand(12345);
    for (size_t i = 0; i < N; i++) data[i] = (uint8_t)(rand() % 26);
    HuffTree t;
    clock_t c0 = clock();
    huff_build(data, N, &t);
    clock_t c1 = clock();
    uint8_t *bits = (uint8_t *)malloc((N * 8 + 7) / 8 + 16);
    size_t nb = huff_encode(data, N, &t, bits, (N * 8 + 7) / 8 + 16);
    clock_t c2 = clock();
    uint8_t *dec = (uint8_t *)malloc(N);
    size_t nd = huff_decode(bits, nb * 8, &t, dec, N);
    clock_t c3 = clock();
    double build_ms = (double)(c1 - c0) / CLOCKS_PER_SEC * 1000.0;
    double enc_ms   = (double)(c2 - c1) / CLOCKS_PER_SEC * 1000.0;
    double dec_ms   = (double)(c3 - c2) / CLOCKS_PER_SEC * 1000.0;
    CHECK(nd == N && memcmp(dec, data, N) == 0, "benchmark roundtrip");
    printf("  (build=%.3f ms, encode=%.3f ms, decode=%.3f ms, %zu bytes -> %zu bytes)\n",
           build_ms, enc_ms, dec_ms, N, nb);
    free(data); free(bits); free(dec);
}

int main(void) {
    printf("=== Huffman Coding Unit Tests ===\n");
    test_empty();
    test_single_symbol();
    test_two_symbols();
    test_roundtrip_text();
    test_all_symbols();
    test_capacity_guard();
    test_benchmark();
    printf("\n%s (%d failures)\n", g_failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED",
           g_failures);
    return g_failures == 0 ? 0 : 1;
}
