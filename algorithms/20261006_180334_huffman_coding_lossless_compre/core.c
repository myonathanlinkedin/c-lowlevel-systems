#include "types.h"
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>

static HuffmanNode *create_node(uint32_t freq, uint8_t symbol, bool is_leaf) {
    HuffmanNode *node = (HuffmanNode *)malloc(sizeof(HuffmanNode));
    if (!node) return NULL;
    node->freq = freq;
    node->symbol = symbol;
    node->left = node->right = NULL;
    node->is_leaf = is_leaf;
    return node;
}

static void free_node(HuffmanNode *node) {
    if (!node) return;
    free_node(node->left);
    free_node(node->right);
    free(node);
}

HuffmanTree *build_huffman_tree(const uint8_t *data, size_t len) {
    uint32_t freq[256] = {0};
    for (size_t i = 0; i < len; ++i) freq[data[i]]++;
    HuffmanNode *nodes[256];
    size_t node_count = 0;
    for (int i = 0; i < 256; ++i) {
        if (freq[i]) {
            nodes[node_count++] = create_node(freq[i], (uint8_t)i, true);
        }
    }
    if (node_count == 0) return NULL;
    if (node_count == 1) {
        HuffmanTree *tree = (HuffmanTree *)malloc(sizeof(HuffmanTree));
        tree->root = nodes[0];
        return tree;
    }
    while (node_count > 1) {
        // Find two smallest nodes
        size_t min1 = 0, min2 = 1;
        if (nodes[min2]->freq < nodes[min1]->freq) { size_t tmp = min1; min1 = min2; min2 = tmp; }
        for (size_t i = 2; i < node_count; ++i) {
            if (nodes[i]->freq < nodes[min1]->freq) { min2 = min1; min1 = i; }
            else if (nodes[i]->freq < nodes[min2]->freq) { min2 = i; }
        }
        HuffmanNode *left = nodes[min1];
        HuffmanNode *right = nodes[min2];
        HuffmanNode *parent = create_node(left->freq + right->freq, 0, false);
        parent->left = left;
        parent->right = right;
        // Replace min1 with parent, remove min2
        nodes[min1] = parent;
        nodes[min2] = nodes[node_count - 1];
        node_count--;
    }
    HuffmanTree *tree = (HuffmanTree *)malloc(sizeof(HuffmanTree));
    tree->root = nodes[0];
    return tree;
}

void free_huffman_tree(HuffmanTree *tree) {
    if (!tree) return;
    free_node(tree->root);
    free(tree);
}

static void generate_codes_rec(const HuffmanNode *node, HuffmanCode *codes, uint8_t code, uint8_t length) {
    if (!node) return;
    if (node->is_leaf) {
        codes[node->symbol].bits = code;
        codes[node->symbol].length = length;
        return;
    }
    generate_codes_rec(node->left, codes, (code << 1), length + 1);
    generate_codes_rec(node->right, codes, (code << 1) | 1, length + 1);
}

void generate_codes(const HuffmanNode *node, HuffmanCode *codes, uint8_t code, uint8_t length) {
    generate_codes_rec(node, codes, code, length);
}

static bool bitwriter_init(BitWriter *bw, size_t capacity) {
    bw->buffer = (uint8_t *)malloc(capacity);
    if (!bw->buffer) return false;
    bw->capacity = capacity;
    bw->size = 0;
    bw->current = 0;
    bw->bits_filled = 0;
    return true;
}

static void bitwriter_free(BitWriter *bw) {
    free(bw->buffer);
    bw->buffer = NULL;
    bw->capacity = 0;
    bw->size = 0;
}

static bool bitwriter_write(BitWriter *bw, uint8_t bits, uint8_t length) {
    for (int i = length - 1; i >= 0; --i) {
        bw->current = (bw->current << 1) | ((bits >> i) & 1);
        bw->bits_filled++;
        if (bw->bits_filled == 8) {
            if (bw->size >= bw->capacity) return false;
            bw->buffer[bw->size++] = bw->current;
            bw->current = 0;
            bw->bits_filled = 0;
        }
    }
    return true;
}

static bool bitwriter_flush(BitWriter *bw) {
    if (bw->bits_filled) {
        bw->current <<= (8 - bw->bits_filled);
        if (bw->size >= bw->capacity) return false;
        bw->buffer[bw->size++] = bw->current;
        bw->current = 0;
        bw->bits_filled = 0;
    }
    return true;
}

bool encode(const uint8_t *data, size_t len, const HuffmanCode *codes, BitWriter *out) {
    if (!bitwriter_init(out, len * 8 + 8)) return false;
    for (size_t i = 0; i < len; ++i) {
        if (!bitwriter_write(out, codes[data[i]].bits, codes[data[i]].length)) {
            bitwriter_free(out);
            return false;
        }
    }
    return bitwriter_flush(out);
}

static bool bitreader_init(BitReader *br, const uint8_t *buffer, size_t size) {
    br->buffer = buffer;
    br->size = size;
    br->pos = 0;
    br->current = 0;
    br->bits_left = 0;
    return true;
}

static bool bitreader_read(BitReader *br, uint8_t *bit) {
    if (br->bits_left == 0) {
        if (br->pos >= br->size) return false;
        br->current = br->buffer[br->pos++];
        br->bits_left = 8;
    }
    *bit = (br->current >> 7) & 1;
    br->current <<= 1;
    br->bits_left--;
    return true;
}

bool decode(const BitReader *in, const HuffmanTree *tree, uint8_t *out, size_t *out_len) {
    size_t idx = 0;
    const HuffmanNode *node = tree->root;
    BitReader br = *in;
    uint8_t bit;
    while (bitreader_read(&br, &bit)) {
        node = bit ? node->right : node->left;
        if (!node) return false;
        if (node->is_leaf) {
            out[idx++] = node->symbol;
            node = tree->root;
        }
    }
    *out_len = idx;
    return true;
}
