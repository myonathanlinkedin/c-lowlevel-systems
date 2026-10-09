#include "types.h"
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>

/* ---------- Priority queue (linear, small N) ---------- */
static void pq_push(HuffmanNode **queue, size_t *size, HuffmanNode *node) {
    queue[(*size)++] = node;
}
static HuffmanNode *pq_pop_min(HuffmanNode **queue, size_t *size) {
    size_t min_idx = 0;
    for (size_t i = 1; i < *size; ++i) {
        if (queue[i]->freq < queue[min_idx]->freq) min_idx = i;
    }
    HuffmanNode *res = queue[min_idx];
    queue[min_idx] = queue[--(*size)];
    return res;
}

/* ---------- Tree construction ---------- */
void huffman_build_tree(const uint32_t freq[256], HuffmanNode **out_root) {
    HuffmanNode *queue[256];
    size_t qsize = 0;

    for (uint16_t i = 0; i < 256; ++i) {
        if (freq[i] == 0) continue;
        HuffmanNode *node = (HuffmanNode *)malloc(sizeof(HuffmanNode));
        node->symbol = (uint8_t)i;
        node->freq   = freq[i];
        node->leaf   = true;
        node->left   = node->right = NULL;
        pq_push(queue, &qsize, node);
    }

    /* Edge case: no symbols -> create a dummy leaf */
    if (qsize == 0) {
        HuffmanNode *node = (HuffmanNode *)malloc(sizeof(HuffmanNode));
        node->symbol = 0;
        node->freq   = 0;
        node->leaf   = true;
        node->left = node->right = NULL;
        *out_root = node;
        return;
    }

    while (qsize > 1) {
        HuffmanNode *a = pq_pop_min(queue, &qsize);
        HuffmanNode *b = pq_pop_min(queue, &qsize);
        HuffmanNode *parent = (HuffmanNode *)malloc(sizeof(HuffmanNode));
        parent->symbol = 0;               // unused
        parent->freq   = a->freq + b->freq;
        parent->leaf   = false;
        parent->left   = a;
        parent->right  = b;
        pq_push(queue, &qsize, parent);
    }
    *out_root = pq_pop_min(queue, &qsize);
}

/* ---------- Tree deallocation ---------- */
void huffman_free_tree(HuffmanNode *root) {
    if (!root) return;
    if (!root->leaf) {
        huffman_free_tree(root->left);
        huffman_free_tree(root->right);
    }
    free(root);
}

/* ---------- Code generation ---------- */
static void gen_codes_rec(const HuffmanNode *node,
                          uint32_t code, uint8_t length,
                          HuffmanCode codes[256]) {
    if (node->leaf) {
        codes[node->symbol].bits   = code;
        codes[node->symbol].length = length ? length : 1; // at least 1 bit
        return;
    }
    /* left = 0, right = 1 */
    gen_codes_rec(node->left,  code << 1,        length + 1, codes);
    gen_codes_rec(node->right, (code << 1) | 1U, length + 1, codes);
}
void huffman_generate_codes(const HuffmanNode *root, HuffmanCode codes[256]) {
    for (size_t i = 0; i < 256; ++i) {
        codes[i].bits = 0;
        codes[i].length = 0;
    }
    gen_codes_rec(root, 0, 0, codes);
}

/* ---------- Bit writer ---------- */
size_t huffman_compress(const uint8_t *input, size_t in_len,
                        uint8_t *output, const HuffmanCode codes[256]) {
    uint8_t cur = 0;
    uint8_t filled = 0;
    size_t out_idx = 0;

    for (size_t i = 0; i < in_len; ++i) {
        HuffmanCode c = codes[input[i]];
        for (uint8_t b = 0; b < c.length; ++b) {
            uint8_t bit = (c.bits >> b) & 1U;          // LSB‑first
            cur |= bit << filled;
            ++filled;
            if (filled == 8) {
                output[out_idx++] = cur;
                cur = 0;
                filled = 0;
            }
        }
    }
    if (filled) {
        output[out_idx++] = cur;   // pad remaining bits with zeros
    }
    return out_idx;
}

/* ---------- Bit reader ---------- */
size_t huffman_decompress(const uint8_t *input, size_t in_len,
                          uint8_t *output, size_t out_len,
                          const HuffmanNode *root) {
    size_t out_idx = 0;
    const HuffmanNode *node = root;
    for (size_t i = 0; i < in_len && out_idx < out_len; ++i) {
        uint8_t cur = input[i];
        for (uint8_t bitpos = 0; bitpos < 8 && out_idx < out_len; ++bitpos) {
            uint8_t bit = (cur >> bitpos) & 1U;
            node = bit ? node->right : node->left;
            if (node->leaf) {
                output[out_idx++] = node->symbol;
                node = root;
            }
        }
    }
    return out_idx;
}
