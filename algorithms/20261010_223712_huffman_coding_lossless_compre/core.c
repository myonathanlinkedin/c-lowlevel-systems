#include "types.h"
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ---------- Helper Functions ---------- */

static HuffNode* node_create(uint8_t symbol, uint32_t freq,
                            HuffNode* left, HuffNode* right)
{
    HuffNode* n = (HuffNode*)malloc(sizeof(HuffNode));
    if (!n) return NULL;
    n->symbol = symbol;
    n->freq = freq;
    n->left = left;
    n->right = right;
    return n;
}

static void node_free(HuffNode* n)
{
    if (!n) return;
    node_free(n->left);
    node_free(n->right);
    free(n);
}

/* Simple O(N^2) priority queue for up to 256 nodes */
static void pq_push(HuffNode** pq, size_t* size, HuffNode* node)
{
    size_t i = (*size)++;
    while (i > 0) {
        size_t parent = (i - 1) / 2;
        if (pq[parent]->freq <= node->freq) break;
        pq[i] = pq[parent];
        i = parent;
    }
    pq[i] = node;
}

static HuffNode* pq_pop(HuffNode** pq, size_t* size)
{
    assert(*size > 0);
    HuffNode* top = pq[0];
    HuffNode* last = pq[--(*size)];
    size_t i = 0;
    while (true) {
        size_t left = 2 * i + 1;
        size_t right = left + 1;
        if (left >= *size) break;
        size_t smallest = left;
        if (right < *size && pq[right]->freq < pq[left]->freq)
            smallest = right;
        if (pq[smallest]->freq >= last->freq) break;
        pq[i] = pq[smallest];
        i = smallest;
    }
    pq[i] = last;
    return top;
}

/* Build Huffman tree from frequency table */
static HuffNode* build_tree(const uint32_t freq[256])
{
    HuffNode* pq[256];
    size_t pq_size = 0;
    for (uint16_t i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            HuffNode* n = node_create((uint8_t)i, freq[i], NULL, NULL);
            if (!n) { /* OOM cleanup */
                while (pq_size) node_free(pq_pop(pq, &pq_size));
                return NULL;
            }
            pq_push(pq, &pq_size, n);
        }
    }
    if (pq_size == 0) {
        /* Empty input: create a dummy leaf for symbol 0 */
        return node_create(0, 0, NULL, NULL);
    }
    while (pq_size > 1) {
        HuffNode* a = pq_pop(pq, &pq_size);
        HuffNode* b = pq_pop(pq, &pq_size);
        uint32_t sum = a->freq + b->freq;
        HuffNode* parent = node_create(0, sum, a, b);
        if (!parent) {
            node_free(a);
            node_free(b);
            while (pq_size) node_free(pq_pop(pq, &pq_size));
            return NULL;
        }
        pq_push(pq, &pq_size, parent);
    }
    return pq_pop(pq, &pq_size);
}

/* Recursively generate codes */
static void generate_codes(const HuffNode* node, HuffCode table[256],
                           uint32_t code, uint8_t length)
{
    if (!node->left && !node->right) {
        table[node->symbol].bits = code;
        table[node->symbol].length = length;
        return;
    }
    if (node->left) {
        generate_codes(node->left, table, (code << 1), length + 1);
    }
    if (node->right) {
        generate_codes(node->right, table, (code << 1) | 1, length + 1);
    }
}

/* ---------- Compression ---------- */

bool huffman_compress(const uint8_t* input, size_t input_len,
                      uint8_t** output, size_t* output_len)
{
    if (!output || !output_len) return false;
    *output = NULL;
    *output_len = 0;

    uint32_t freq[256] = {0};
    for (size_t i = 0; i < input_len; ++i) {
        ++freq[input[i]];
    }

    HuffNode* root = build_tree(freq);
    if (!root) return false;

    HuffCode table[256] = {{0,0}};
    generate_codes(root, table, 0, 0);
    node_free(root);

    /* Estimate worst-case size: header (1024 + 4) + input_len * 4 (max 32 bits per byte) */
    size_t est_size = 1024 + 4 + input_len * 4;
    uint8_t* out_buf = (uint8_t*)malloc(est_size);
    if (!out_buf) return false;

    /* Write frequency table (256 * 4 bytes, little endian) */
    uint8_t* p = out_buf;
    for (size_t i = 0; i < 256; ++i) {
        uint32_t f = freq[i];
        *p++ = (uint8_t)(f & 0xFF);
        *p++ = (uint8_t)((f >> 8) & 0xFF);
        *p++ = (uint8_t)((f >> 16) & 0xFF);
        *p++ = (uint8_t)((f >> 24) & 0xFF);
    }
    /* Original length */
    uint32_t orig_len = (uint32_t)input_len;
    *p++ = (uint8_t)(orig_len & 0xFF);
    *p++ = (uint8_t)((orig_len >> 8) & 0xFF);
    *p++ = (uint8_t)((orig_len >> 16) & 0xFF);
    *p++ = (uint8_t)((orig_len >> 24) & 0xFF);

    /* Bit writer */
    uint8_t cur_byte = 0;
    uint8_t bits_filled = 0;

    for (size_t i = 0; i < input_len; ++i) {
        HuffCode c = table[input[i]];
        uint32_t bits = c.bits;
        uint8_t len = c.length;
        for (int8_t b = len - 1; b >= 0; --b) {
            uint8_t bit = (bits >> b) & 1U;
            cur_byte = (uint8_t)((cur_byte << 1) | bit);
            ++bits_filled;
            if (bits_filled == 8) {
                *p++ = cur_byte;
                cur_byte = 0;
                bits_filled = 0;
            }
        }
    }
    if (bits_filled) {
        cur_byte <<= (8 - bits_filled);
        *p++ = cur_byte;
    }

    size_t final_size = (size_t)(p - out_buf);
    uint8_t* resized = (uint8_t*)realloc(out_buf, final_size);
    if (!resized) {
        free(out_buf);
        return false;
    }
    *output = resized;
    *output_len = final_size;
    return true;
}

/* ---------- Decompression ---------- */

static HuffNode* rebuild_tree_from_freq(const uint32_t freq[256])
{
    return build_tree(freq);
}

/* Read 4-byte little-endian integer */
static uint32_t read_u32_le(const uint8_t* src)
{
    return ((uint32_t)src[0]) |
           ((uint32_t)src[1] << 8) |
           ((uint32_t)src[2] << 16) |
           ((uint32_t)src[3] << 24);
}

bool huffman_decompress(const uint8_t* input, size_t input_len,
                        uint8_t** output, size_t* output_len)
{
    if (!input || input_len < 1024 + 4 || !output || !output_len) return false;
    const uint8_t* p = input;

    uint32_t freq[256];
    for (size_t i = 0; i < 256; ++i) {
        freq[i] = read_u32_le(p);
        p += 4;
    }
    uint32_t orig_len = read_u32_le(p);
    p += 4;

    HuffNode* root = rebuild_tree_from_freq(freq);
    if (!root) return false;

    uint8_t* out_buf = (uint8_t*)malloc(orig_len ? orig_len : 1);
    if (!out_buf) {
        node_free(root);
        return false;
    }

    size_t out_pos = 0;
    HuffNode* cur = root;
    while (out_pos < orig_len && (size_t)(p - input) < input_len) {
        uint8_t byte = *p++;
        for (int bit = 7; bit >= 0 && out_pos < orig_len; --bit) {
            uint8_t b = (byte >> bit) & 1U;
            cur = b ? cur->right : cur->left;
            if (!cur) { /* malformed tree */
                free(out_buf);
                node_free(root);
                return false;
            }
            if (!cur->left && !cur->right) {
                out_buf[out_pos++] = cur->symbol;
                cur = root;
            }
        }
    }

    node_free(root);
    if (out_pos != orig_len) {
        free(out_buf);
        return false;
    }
    *output = out_buf;
    *output_len = orig_len;
    return true;
}

void huffman_free(uint8_t* buffer)
{
    free(buffer);
}
