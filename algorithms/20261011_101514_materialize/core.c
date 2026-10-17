#include "types.h"
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>

#include "core.h"
#include <stdlib.h>
#include <assert.h>

static TreeNode* create_node(int value) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    assert(node != NULL);
    node->value = value;
    node->left = NULL;
    node->right = NULL;
    return node;
}

/* Find the index of `value` in `inorder` between [start, end). */
static size_t find_inorder_index(const int* inorder, size_t start, size_t end, int value) {
    for (size_t i = start; i < end; ++i) {
        if (inorder[i] == value) {
            return i;
        }
    }
    assert(0 && "Value not found in inorder segment");
    return end; /* Unreachable */
}

/* Recursive helper that builds the tree.
   `pre_idx` points to the current element in preorder array. */
static TreeNode* build_recursive(const int* preorder, size_t* pre_idx,
                                 const int* inorder, size_t in_start, size_t in_end) {
    if (in_start >= in_end) {
        return NULL;
    }
    int root_val = preorder[*pre_idx];
    (*pre_idx)++;
    TreeNode* root = create_node(root_val);
    size_t in_root_idx = find_inorder_index(inorder, in_start, in_end, root_val);
    root->left = build_recursive(preorder, pre_idx, inorder, in_start, in_root_idx);
    root->right = build_recursive(preorder, pre_idx, inorder, in_root_idx + 1, in_end);
    return root;
}

TreeNode* materialize_tree(const int* preorder, const int* inorder, size_t length) {
    if (length == 0) {
        return NULL;
    }
    size_t pre_idx = 0;
    return build_recursive(preorder, &pre_idx, inorder, 0, length);
}

void free_tree(TreeNode* root) {
    if (root == NULL) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

void inorder_collect(const TreeNode* root, int* out, size_t* idx) {
    if (root == NULL) return;
    inorder_collect(root->left, out, idx);
    out[*idx] = root->value;
    (*idx)++;
    inorder_collect(root->right, out, idx);
}

void preorder_collect(const TreeNode* root, int* out, size_t* idx) {
    if (root == NULL) return;
    out[*idx] = root->value;
    (*idx)++;
    preorder_collect(root->left, out, idx);
    preorder_collect(root->right, out, idx);
}
