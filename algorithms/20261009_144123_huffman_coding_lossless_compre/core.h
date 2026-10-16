#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#pragma once
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Build a Huffman tree from symbol frequencies. */
void huffman_build_tree(const uint32_t freq[256], HuffmanNode **out_root);

/* Free the tree allocated by huffman_build_tree. */
void huffman_free_tree(HuffmanNode *root);

/* Fill 'codes' with Huffman codes derived from the tree. */
void huffman_generate_codes(const HuffmanNode *root, HuffmanCode codes[256]);

/* Compress 'in_len' bytes from 'input' into 'output'.
   Returns number of bytes written to 'output'. */
size_t huffman_compress(const uint8_t *input, size_t in_len,
                        uint8_t *output, const HuffmanCode codes[256]);

/* Decompress 'in_len' bytes from 'input' into 'output' (max 'out_len' bytes).
   Returns number of bytes written to 'output'. */
size_t huffman_decompress(const uint8_t *input, size_t in_len,
                          uint8_t *output, size_t out_len,
                          const HuffmanNode *root);

#ifdef __cplusplus
}
#endif
