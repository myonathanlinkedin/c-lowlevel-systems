#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Compute stride vector according to the layout's ordering. */
static void compute_strides(Layout *l) {
    assert(l != NULL);
    if (l->row_major) {
        /* C row‑major: last dimension stride = 1 */
        l->strides[l->ndim - 1] = 1;
        for (ssize_t i = (ssize_t)l->ndim - 2; i >= 0; --i) {
            l->strides[i] = l->strides[i + 1] * l->dims[i + 1];
        }
    } else {
        /* Column‑major: first dimension stride = 1 */
        l->strides[0] = 1;
        for (size_t i = 1; i < l->ndim; ++i) {
            l->strides[i] = l->strides[i - 1] * l->dims[i - 1];
        }
    }
}

/* Allocate and initialise a Layout descriptor. */
Layout* layout_create(size_t ndim, const size_t *dims, bool row_major) {
    if (ndim == 0 || dims == NULL) {
        return NULL;
    }
    Layout *l = (Layout*)malloc(sizeof(Layout));
    if (!l) return NULL;

    l->ndim = ndim;
    l->row_major = row_major;
    l->dims = (size_t*)malloc(ndim * sizeof(size_t));
    l->strides = (size_t*)malloc(ndim * sizeof(size_t));
    if (!l->dims || !l->strides) {
        free(l->dims);
        free(l->strides);
        free(l);
        return NULL;
    }
    memcpy(l->dims, dims, ndim * sizeof(size_t));
    compute_strides(l);
    return l;
}

/* Release resources owned by a Layout. */
void layout_destroy(Layout *l) {
    if (!l) return;
    free(l->dims);
    free(l->strides);
    free(l);
}

/* Verify that all dimensions are non‑zero. */
bool layout_is_valid(const Layout *l) {
    if (!l || l->ndim == 0) return false;
    for (size_t i = 0; i < l->ndim; ++i) {
        if (l->dims[i] == 0) return false;
    }
    return true;
}

/* Convert multi‑dimensional indices to a linear offset. */
size_t layout_offset(const Layout *l, const size_t *indices) {
    assert(l != NULL && indices != NULL);
    size_t offset = 0;
    for (size_t i = 0; i < l->ndim; ++i) {
        assert(indices[i] < l->dims[i]);
        offset += indices[i] * l->strides[i];
    }
    return offset;
}

/* Produce a transposed layout according to a permutation vector. */
Layout* layout_transpose(const Layout *l, const size_t *perm) {
    assert(l != NULL && perm != NULL);
    Layout *t = (Layout*)malloc(sizeof(Layout));
    if (!t) return NULL;

    t->ndim = l->ndim;
    t->row_major = l->row_major;
    t->dims = (size_t*)malloc(t->ndim * sizeof(size_t));
    t->strides = (size_t*)malloc(t->ndim * sizeof(size_t));
    if (!t->dims || !t->strides) {
        free(t->dims);
        free(t->strides);
        free(t);
        return NULL;
    }

    for (size_t i = 0; i < t->ndim; ++i) {
        size_t p = perm[i];
        assert(p < l->ndim);
        t->dims[i] = l->dims[p];
    }
    compute_strides(t);
    return t;
}

/* Reshape a layout while preserving total element count. */
Layout* layout_reshape(const Layout *l, size_t new_ndim, const size_t *new_dims) {
    assert(l != NULL && new_dims != NULL);
    size_t old_total = 1;
    for (size_t i = 0; i < l->ndim; ++i) {
        old_total *= l->dims[i];
    }
    size_t new_total = 1;
    for (size_t i = 0; i < new_ndim; ++i) {
        new_total *= new_dims[i];
    }
    if (old_total != new_total) return NULL;
    return layout_create(new_ndim, new_dims, l->row_major);
}
