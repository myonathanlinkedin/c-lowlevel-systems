
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdatomic.h>
#include <limits.h>

typedef struct Node {
    int key;
    int value;
    _Atomic unsigned int lock;   // version lock: even = unlocked, odd = locked
    struct Node *next;
} Node;

typedef struct List {
    Node *head;                  // sentinel node with key = INT_MIN
} List;
