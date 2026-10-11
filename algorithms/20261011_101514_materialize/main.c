#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "types.h"
#include "core.h"

static void test_materialize(const int* preorder, const int* inorder, size_t n) {
    TreeNode* root = materialize_tree(preorder, inorder, n);
    assert(root != NULL || n == 0);

    int* in_out = (int*)malloc(n * sizeof(int));
    int* pre_out = (int*)malloc(n * sizeof(int));
    assert(in_out && pre_out);

    size_t idx = 0;
    inorder_collect(root, in_out, &idx);
    assert(idx == n);
    idx = 0;
    preorder_collect(root, pre_out, &idx);
    assert(idx == n);

    for (size_t i = 0; i < n; ++i) {
        assert(in_out[i] == inorder[i]);
        assert(pre_out[i] == preorder[i]);
    }

    free(in_out);
    free(pre_out);
    free_tree(root);
}

int main(void) {
    /* Test 1: simple three‑node tree */
    int preorder1[] = {1, 2, 3};
    int inorder1[]  = {2, 1, 3};
    test_materialize(preorder1, inorder1, 3);

    /* Test 2: left‑skewed tree */
    int preorder2[] = {4, 3, 2, 1};
    int inorder2[]  = {1, 2, 3, 4};
    test_materialize(preorder2, inorder2, 4);

    /* Test 3: right‑skewed tree */
    int preorder3[] = {5, 6, 7, 8};
    int inorder3[]  = {5, 6, 7, 8};
    test_materialize(preorder3, inorder3, 4);

    /* Test 4: balanced tree */
    int preorder4[] = {10, 5, 2, 7, 15, 12, 20};
    int inorder4[]  = {2, 5, 7, 10, 12, 15, 20};
    test_materialize(preorder4, inorder4, 7);

    /* Test 5: empty tree */
    test_materialize(NULL, NULL, 0);

    printf("All Materialize tests passed.\n");
    return 0;
}
