#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_SYMBOLS 256
#define MAX_NODES (MAX_SYMBOLS * 2 - 1)

typedef struct {
    uint32_t freq;
    uint8_t  sym;
    int      left, right;
} Node;

typedef struct {
    uint8_t  buf;
    int      pos;
} BitWriter;

typedef struct {
    const uint8_t *buf;
    size_t         len;
    size_t         pos;
    int            pos_bit;
} BitReader;

static void bw_init(BitWriter *bw) { bw->buf = 0; bw->pos = 0; }
static void bw_write(BitWriter *bw, uint32_t bits, int len, uint8_t *out) {
    for (int i = len-1; i >= 0; --i) {
        bw->buf = (bw->buf << 1) | ((bits >> i) & 1);
        bw->pos++;
        if (bw->pos == 8) {
            out[bw->pos/8 - 1] = bw->buf;
            bw->buf = 0;
            bw->pos = 0;
        }
    }
}
static void bw_flush(BitWriter *bw, uint8_t *out, int *pad) {
    if (bw->pos) {
        bw->buf <<= (8 - bw->pos);
        out[bw->pos/8] = bw->buf;
        *pad = 8 - bw->pos;
    } else *pad = 0;
}

static void br_init(BitReader *br, const uint8_t *buf, size_t len) {
    br->buf = buf; br->len = len; br->pos = 0; br->pos_bit = 0;
}
static int br_read(BitReader *br, uint8_t *bit) {
    if (br->pos >= br->len) return 0;
    *bit = (br->buf[br->pos] >> (7 - br->pos_bit)) & 1;
    br->pos_bit++;
    if (br->pos_bit == 8) { br->pos_bit = 0; br->pos++; }
    return 1;
}

static void build_tree(Node *nodes, uint32_t *freq, int *root) {
    int node_cnt = 0;
    for (int i = 0; i < MAX_SYMBOLS; ++i) {
        if (freq[i]) {
            nodes[node_cnt].freq = freq[i];
            nodes[node_cnt].sym  = (uint8_t)i;
            nodes[node_cnt].left = nodes[node_cnt].right = -1;
            node_cnt++;
        }
    }
    if (node_cnt == 1) { *root = 0; return; }
    while (node_cnt > 1) {
        int a = -1, b = -1;
        for (int i = 0; i < node_cnt; ++i) {
            if (a==-1 || nodes[i].freq < nodes[a].freq) a = i;
        }
        for (int i = 0; i < node_cnt; ++i) {
            if (i==a) continue;
            if (b==-1 || nodes[i].freq < nodes[b].freq) b = i;
        }
        Node newnode = { nodes[a].freq + nodes[b].freq, 0, a, b };
        nodes[node_cnt++] = newnode;
        for (int i = 0; i < node_cnt-1; ++i) {
            if (i==a || i==b) continue;
            if (nodes[i].freq == 0) { nodes[i] = nodes[node_cnt-2]; nodes[node_cnt-2].freq = 0; }
        }
    }
    *root = node_cnt-1;
}

static void gen_codes(Node *nodes, int node, uint32_t code, int len, uint32_t *codes, uint8_t *code_len) {
    if (nodes[node].left==-1 && nodes[node].right==-1) {
        codes[nodes[node].sym] = code;
        code_len[nodes[node].sym] = len;
        return;
    }
    if (nodes[node].left!=-1) gen_codes(nodes, nodes[node].left, code<<1, len+1, codes, code_len);
    if (nodes[node].right!=-1) gen_codes(nodes, nodes[node].right, (code<<1)|1, len+1, codes, code_len);
}

int compress(const uint8_t *in, size_t inlen, uint8_t **out, size_t *outlen) {
    uint32_t freq[MAX_SYMBOLS] = {0};
    for (size_t i=0;i<inlen;i++) freq[in[i]]++;
    Node nodes[MAX_NODES];
    int root;
    build_tree(nodes, freq, &root);
    uint32_t codes[MAX_SYMBOLS] = {0};
    uint8_t  code_len[MAX_SYMBOLS] = {0};
    gen_codes(nodes, root, 0, 0, codes, code_len);
    size_t header_size = MAX_SYMBOLS * 4 + 1;
    size_t est_bits = 0;
    for (size_t i=0;i<inlen;i++) est_bits += code_len[in[i]];
    size_t est_bytes = header_size + (est_bits + 7)/8 + 1;
    uint8_t *buf = malloc(est_bytes);
    if (!buf) return -1;
    // write header frequencies
    for (int i=0;i<MAX_SYMBOLS;i++) {
        uint32_t f = freq[i];
        buf[i*4+0] = (f>>24)&0xFF;
        buf[i*4+1] = (f>>16)&0xFF;
        buf[i*4+2] = (f>>8)&0xFF;
        buf[i*4+3] = f&0xFF;
    }
    BitWriter bw; bw_init(&bw);
    for (size_t i=0;i<inlen;i++) bw_write(&bw, codes[in[i]], code_len[in[i]], buf+header_size);
    int pad;
    bw_flush(&bw, buf+header_size, &pad);
    buf[header_size + (est_bits+7)/8] = (uint8_t)pad;
    *out = buf;
    *outlen = header_size + (est_bits+7)/8 + 1;
    return 0;
}

int decompress(const uint8_t *in, size_t inlen, uint8_t **out, size_t *outlen) {
    if (inlen < MAX_SYMBOLS*4+1) return -1;
    uint32_t freq[MAX_SYMBOLS];
    for (int i=0;i<MAX_SYMBOLS;i++) {
        freq[i] = (in[i*4+0]<<24)|(in[i*4+1]<<16)|(in[i*4+2]<<8)|(in[i*4+3]);
    }
    Node nodes[MAX_NODES];
    int root;
    build_tree(nodes, freq, &root);
    int pad = in[MAX_SYMBOLS*4];
    size_t data_bits = (inlen - MAX_SYMBOLS*4 - 1)*8 - pad;
    BitReader br; br_init(&br, in+MAX_SYMBOLS*4+1, inlen - MAX_SYMBOLS*4 - 1);
    size_t out_cap = data_bits; // rough estimate
    uint8_t *buf = malloc(out_cap);
    if (!buf) return -1;
    size_t out_pos = 0;
    int node = root;
    for (size_t i=0;i<data_bits;i++) {
        uint8_t bit; br_read(&br,&bit);
        node = bit ? nodes[node].right : nodes[node].left;
        if (nodes[node].left==-1 && nodes[node].right==-1) {
            buf[out_pos++] = nodes[node].sym;
            node = root;
        }
    }
    *out = buf;
    *outlen = out_pos;
    return 0;
}

int main(void) {
    const char *text = "this is an example for huffman encoding";
    size_t inlen = strlen(text);
    uint8_t *comp, *decomp;
    size_t comp_len, decomp_len;
    if (compress((const uint8_t*)text, inlen, &comp, &comp_len) != 0) return 1;
    if (decompress(comp, comp_len, &decomp, &decomp_len) != 0) return 1;
    if (decomp_len != inlen || memcmp(text, decomp, inlen) != 0) {
        fprintf(stderr, "Mismatch!\n");
        return 1;
    }
    printf("Original: %s\n", text);
    printf("Compressed size: %zu bytes\n", comp_len);
    printf("Decompressed matches original.\n");
    free(comp); free(decomp);
    return 0;
}
