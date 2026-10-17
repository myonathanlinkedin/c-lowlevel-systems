#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>

#pragma once
#include "types.h"

TreeNode* materialize_tree(const int* preorder, const int* inorder, size_t length);
void free_tree(TreeNode* root);
void inorder_collect(const TreeNode* root, int* out, size_t* idx);
void preorder_collect(const TreeNode* root, int* out, size_t* idx);
