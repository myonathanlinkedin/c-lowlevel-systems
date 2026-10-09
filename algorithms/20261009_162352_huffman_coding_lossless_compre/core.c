#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "types.h"
#include "core.h"
#include <string.h>
#include <stdlib.h>

static int cmp_freq(const void *a, const void *b) {
    const HuffNode *x = (const HuffNode *)a;
    const HuffNode *y = (const HuffNode *)b;
    if (x->freq < y->freq) return -1;
    if (x->freq > y->freq) return 1;
    return 0;
}

static void assign_codes(const HuffNode *nodes, int idx,
                         const uint8_t *prefix, uint8_t depth,
                         uint8_t codes[HUFF_ALPHABET][32],
                         uint8_t lengths[HUFF_ALPHABET]) {
    if (idx < 0) return;
    if (nodes[idx].is_leaf) {
        int s = nodes[idx].symbol;
        for (uint8_t i = 0; i < depth; i++) codes[s][i] = prefix[i];
        lengths[s] = depth;
        return;
    }
    uint8_t p[32];
    for (uint8_t i = 0; i < depth; i++) p[i] = prefix[i];
    p[depth] = 0;
    assign_codes(nodes, nodes[idx].left, p, (uint8_t)(depth + 1), codes, lengths);
    p[depth] = 1;
    assign_codes(nodes, nodes[idx].right, p, (uint8_t)(depth + 1), codes, lengths);
}

void huff_build(const uint8_t *data, size_t len, HuffTree *tree) {
    memset(tree, 0, sizeof(*tree));
    uint32_t freq[HUFF_ALPHABET] = {0};
    for (size_t i = 0; i < len; i++) freq[data[i]]++;

    int n = 0;
    for (int s = 0; s < HUFF_ALPHABET; s++) {
        if (freq[s] > 0) {
            HuffNode *nd = &tree->nodes[n];
            nd->freq = freq[s];
            nd->left = -1;
            nd->right = -1;
            nd->symbol = (int16_t)s;
            nd->is_leaf = true;
            n++;
        }
    }
    tree->count = n;
    if (n == 0) { tree->root = -1; return; }

    if (n == 1) {
        HuffNode *nd = &tree->nodes[0];
        nd->left = -1;
        nd->right = -1;
        nd->symbol = (int16_t)0;
        nd->is_leaf = true;
        nd->freq = 1;
        tree->count = 1;
        tree->root = 0;
        for (int s = 0; s < HUFF_ALPHABET; s++) {
            tree->codes[s][0] = 0;
            tree->lengths[s] = (s == 0) ? 1 : 0;
        }
        return;
    }

    qsort(tree->nodes, (size_t)n, sizeof(HuffNode), cmp_freq);

    int next = n;
    int i = 0, j = 0;
    while (next < HUFF_MAX_NODES) {
        HuffNode *a = &tree->nodes[i];
        HuffNode *b = &tree->nodes[j];
        HuffNode *m = &tree->nodes[next];
        m->freq = a->freq + b->freq;
        m->left = (int16_t)i;
        m->right = (int16_t)j;
        m->symbol = -1;
        m->is_leaf = false;
        next++;
        i++;
        j++;
        int k = next - 1;
        while (k > j && tree->nodes[k].freq < tree->nodes[j].freq) {
            HuffNode tmp = tree->nodes[k];
            tree->nodes[k] = tree->nodes[j];
            tree->nodes[j] = tmp;
            k--;
        }
    }
    tree->count = next;
    tree->root = next - 1;

    uint8_t prefix[32] = {0};
    assign_codes(tree->nodes, tree->root, prefix, 0, tree->codes, tree->lengths);
}

size_t huff_bit_length(const uint8_t *data, size_t len, const HuffTree *tree) {
    size_t total = 0;
    for (size_t i = 0; i < len; i++) total += tree->lengths[data[i]];
    return total;
}

size_t huff_encode(const uint8_t *data, size_t len, const HuffTree *tree,
                   uint8_t *out, size_t out_cap) {
    size_t total = huff_bit_length(data, len, tree);
    size_t bytes = (total + 7) / 8;
    if (bytes > out_cap) return 0;
    memset(out, 0, bytes);
    size_t bitpos = 0;
    for (size_t i = 0; i < len; i++) {
        int s = data[i];
        uint8_t L = tree->lengths[s];
        for (uint8_t b = 0; b < L; b++) {
            if (tree->codes[s][b]) {
                size_t byte_idx = bitpos >> 3;
                unsigned shift = (unsigned)(7 - (bitpos & 7u));
                out[byte_idx] |= (uint8_t)(1u << shift);
            }
            bitpos++;
        }
    }
    return bytes;
}

size_t huff_decode(const uint8_t *bits, size_t bit_len, const HuffTree *tree,
                   uint8_t *out, size_t out_cap) {
    if (tree->root < 0) return 0;
    size_t produced = 0;
    int idx = tree->root;
    size_t bitpos = 0;
    while (bitpos < bit_len) {
        size_t byte_idx = bitpos >> 3;
        unsigned shift = (unsigned)(7 - (bitpos & 7u));
        int bit = (bits[byte_idx] >> shift) & 1;
        idx = bit ? tree->nodes[idx].right : tree->nodes[idx].left;
        bitpos++;
        if (tree->nodes[idx].is_leaf) {
            if (produced >= out_cap) break;
            out[produced++] = (uint8_t)tree->nodes[idx].symbol;
            idx = tree->root;
        }
    }
    return produced;
}
