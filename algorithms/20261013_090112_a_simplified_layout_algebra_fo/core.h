#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdint.h>

#pragma once
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Create a layout descriptor.
 *  ndim      – number of dimensions (must be >0)
 *  dims      – array of dimension sizes (length ndim)
 *  row_major – true for C‑style row‑major, false for column‑major
 * Returns NULL on failure.
 */
Layout* layout_create(size_t ndim, const size_t *dims, bool row_major);

/* Release a layout created by layout_create or related functions. */
void layout_destroy(Layout *l);

/* Compute linear offset (in elements) for a given index vector.
 *  indices – array of length l->ndim, each index < corresponding dimension.
 * Returns offset as size_t.
 */
size_t layout_offset(const Layout *l, const size_t *indices);

/* Validate that a layout descriptor is well‑formed. */
bool layout_is_valid(const Layout *l);

/* Produce a transposed layout according to a permutation vector.
 *  perm – array of length l->ndim containing a permutation of {0,…,ndim‑1}.
 * Returns a newly allocated Layout or NULL on failure.
 */
Layout* layout_transpose(const Layout *l, const size_t *perm);

/* Reshape a layout to a new dimensionality.
 *  new_ndim – number of dimensions in the target shape.
 *  new_dims – array of length new_ndim describing the new shape.
 * Returns a newly allocated Layout or NULL if total element count differs.
 */
Layout* layout_reshape(const Layout *l, size_t new_ndim, const size_t *new_dims);

#ifdef __cplusplus
}
#endif
